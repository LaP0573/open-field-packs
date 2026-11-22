#!/usr/bin/env python3
"""Build the packs of this repository with open-field's tools and headers, and hold them to the kept files.

  python -I build.py --open-field <your checkout of open-field> [--out out] [--update]
  python -I build.py --open-field <your checkout of open-field> --make [--display <your unit's pictures>]

Run it from anywhere. It needs a checkout of open-field that has the commit open-field.json names, and the toolchain
in that checkout's folder open-xy/ (open-field's docs/build.md says how to get it), or where --open-xy names it. It
needs no firmware file and reads none.

A pack of this repository is built as any pack by someone else is: with open-field's tools/, its linker scripts and
the headers of the base, and nothing else of open-field. They are taken out of the commit open-field.json names,
never out of the checkout's working files, so the same sources give the same bytes whatever the checkout stands at.

What it does:

  1. makes a work folder, <out>/work: the tools, the linker scripts and the named headers of that commit, the
     toolchain, and this repository's packs beside them, each folder at its own path. The files of shared/ are put
     beside the headers too, so that a pack's #include finds them by name;
  2. holds each pack's table of changes to the pack's own profile (tools/table_build.py), and the page of the
     firmware's functions, firmware-interfaces.md, to the sources and firmware-interfaces.json (tools/interfaces.py);
  3. builds the packs in one run (tools/pack_build.py build3), since fx train and lfo, and 8 tracks and sidechain,
     offer each other names. The run holds each card and each directory of pictures to its recipe;
  4. holds each result to built.json, and to the finished file at the top of this repository.

A pack's folder may hold icon.svg, its picture on the install page. Every build holds it to the form the page's own
pictures have (see icons below); a pack without one is shown with its category's picture.

Answers 0 when every file is the kept one, 1 when one is not, 2 when it could not build. It writes only under the
folder --out names, which must be new or empty, and with the two options below.

`--update` is for a change that is meant to change a pack. It writes the new size and SHA-256 into built.json and
the new file over the one at the top. It refuses a changed file whose version did not grow: give the pack a new
version in its recipe.json, then run --make.

`--make` writes the files that are made from others, in place: each table from the pack's profile.json, each card
and each directory of pictures from the pack's recipe.json, and firmware-interfaces.md from the sources and
firmware-interfaces.json. A card that takes the place of the unit's own pictures
is held to them, so --display names a copy of your unit's folder content/display, kept outside this repository.

Standard library only.
"""

import argparse
import hashlib
import importlib.util
import io
import json
import os
import shutil
import subprocess
import sys
import tarfile

sys.dont_write_bytecode = True
ROOT = os.path.dirname(os.path.abspath(__file__))
KEPT = os.path.join(ROOT, "built.json")
NAMED = os.path.join(ROOT, "open-field.json")
PAGE, WORDS = "firmware-interfaces.md", "firmware-interfaces.json"
OF_THE_FIRMWARE = ("firmware_version", "stock_sha256", "module_base", "reserved_bytes")       # what every profile here says alike


class Problem(Exception):
    pass


def read(path):
    with open(path, "rb") as f:
        return f.read()


def read_json(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def taken_from_open_field(checkout, named, work):
    """The tools, the linker scripts and the named headers of the named commit, written into the work folder."""
    commit = named["commit"]
    there = subprocess.run(["git", "-C", checkout, "cat-file", "-e", commit + "^{commit}"], capture_output=True)
    if there.returncode:
        raise Problem("%s has no commit %s of open-field: run git fetch there" % (checkout, commit))
    paths = ["tools", "linker"] + named["headers"]
    done = subprocess.run(["git", "-C", checkout, "-c", "core.autocrlf=false", "archive", "--format=tar", commit, "--"] + paths, capture_output=True)
    if done.returncode:
        raise Problem("git archive of open-field: %s" % done.stderr.decode("utf-8", "replace").strip())
    with tarfile.open(fileobj=io.BytesIO(done.stdout)) as archive:
        archive.extractall(work, **({"filter": "data"} if hasattr(tarfile, "data_filter") else {}))      # git's own archive


def put_toolchain(source, work):
    for part in ("tooling", os.path.join("local", "toolchain")):
        if not os.path.isdir(os.path.join(source, part)):
            raise Problem("%s is not there: open-field's docs/build.md says how to get the toolchain" % os.path.join(source, part))
        shutil.copytree(os.path.join(source, part), os.path.join(work, "open-xy", part), ignore=shutil.ignore_patterns("__pycache__"))


def put_packs(named, work):
    for folder in named["packs"] + [named["shared"]]:
        shutil.copytree(os.path.join(ROOT, folder), os.path.join(work, folder))
    shared = os.path.join(ROOT, named["shared"])
    for name in sorted(os.listdir(shared)):
        if os.path.isfile(os.path.join(shared, name)):
            target = os.path.join(work, "module", name)
            if os.path.exists(target):
                raise Problem("%s/%s has the name of a header of open-field" % (named["shared"], name))
            shutil.copyfile(os.path.join(shared, name), target)


ICON = "icon.svg"
ICON_SHAPES = {"path": {"d"}, "circle": {"cx", "cy", "r"}, "ellipse": {"cx", "cy", "rx", "ry"},
               "rect": {"x", "y", "width", "height", "rx"}, "g": set()}
ICON_EVERY = {"class", "transform", "stroke-width", "stroke-dasharray", "stroke-opacity"}
ICON_CLASSES = {"fb", "fo", "fr", "fi", "fp", "sb", "sr", "so", "sl"}     # the install page's own: its blue, ochre, red, ink, paper


def held_icons(named):
    """Each pack's icon.svg, held to the install page's form: one <svg> of 120 x 96, drawn with the page's shapes and
    classes and no colour, style, text, link or script of its own. [(the pack's folder, what the page draws)]."""
    import xml.etree.ElementTree as ET
    found = []
    for folder in named["packs"]:
        path = os.path.join(ROOT, folder, ICON)
        if not os.path.isfile(path):
            continue
        data = read(path)
        if len(data) > 4096:
            raise Problem("%s/%s is over 4096 bytes" % (folder, ICON))
        top = ET.fromstring(data)
        if top.tag != "{http://www.w3.org/2000/svg}svg" or top.attrib != {"viewBox": "0 0 120 96"}:
            raise Problem("%s/%s: one <svg viewBox=\"0 0 120 96\">, with nothing else on it" % (folder, ICON))
        for node in top.iter():
            if node is top:
                continue
            tag = node.tag.split("}")[-1]
            if tag not in ICON_SHAPES:
                raise Problem("%s/%s: <%s> is not one of the page's shapes" % (folder, ICON, tag))
            extra = set(node.attrib) - ICON_SHAPES[tag] - ICON_EVERY
            if extra:
                raise Problem("%s/%s: <%s> has %s" % (folder, ICON, tag, ", ".join(sorted(extra))))
            if node.get("class", "fb") not in ICON_CLASSES:
                raise Problem("%s/%s: class %s is not one of the page's" % (folder, ICON, node.get("class")))
            if (node.text or "").strip():
                raise Problem("%s/%s: <%s> holds text" % (folder, ICON, tag))
        inner = data.decode("utf-8").strip()
        found.append((folder, inner[inner.index(">") + 1:inner.rindex("</svg>")]))
    return found


def stage(args, out):
    named = read_json(NAMED)
    work = os.path.join(out, "work")
    os.makedirs(os.path.join(work, "build"))
    taken_from_open_field(args.open_field, named, work)
    put_toolchain(args.open_xy or os.path.join(args.open_field, "open-xy"), work)
    put_packs(named, work)
    return named, work


def functions_page(named, work):
    """The page of the firmware's functions these sources bind, by open-field's tools/interfaces.py: its bytes."""
    with open(os.path.join(work, "build.json"), "w", encoding="utf-8") as f:
        json.dump({"profile": named["packs"][0] + "/profile.json"}, f)          # that tool asks for one when it is loaded
    finder = tool_of(work, "interfaces")
    sources = []
    for folder in named["packs"] + [named["shared"]]:
        for name in sorted(os.listdir(os.path.join(work, folder))):
            if name.endswith((".cpp", ".h")):
                sources.append(("%s/%s" % (folder, name), read(os.path.join(work, folder, name)).decode("utf-8")))
    finder.WORDS = os.path.join(finder.ROOT, WORDS)                             # the page names its words by this path
    page = finder.render(finder.bindings_in(sources), read_json(os.path.join(ROOT, WORDS)))
    return page.replace("Made by `tools/interfaces.py`", "Made by open-field's `tools/interfaces.py`").encode("utf-8")


def tool_of(work, name):
    spec = importlib.util.spec_from_file_location("packs_" + name, os.path.join(work, "tools", name + ".py"))
    tool = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(tool)
    return tool


def run(work, words, log):
    log.write("\n$ python -I %s\n" % " ".join(words))
    log.flush()
    done = subprocess.run([sys.executable, "-I", "-B"] + words, cwd=work, stdout=log, stderr=subprocess.STDOUT)
    if done.returncode:
        raise Problem("%s answered %d: %s says why" % (" ".join(words[:2]), done.returncode, log.name))


def make(named, work, log, display):
    """The made files written again in the work folder, and copied back where they differ: [what changed]."""
    made = []
    for folder in named["packs"]:
        profile = folder + "/profile.json"
        run(work, ["tools/table_build.py", "--profile", profile, "--all"], log)
        made += [table["file"] for table in read_json(os.path.join(work, profile))["tables"]]
        recipe = folder + "/recipe.json"
        for command in ("pictures", "card"):
            for source in read_json(os.path.join(work, recipe))["sources"]:
                if source.endswith("_%s.s" % command):
                    os.remove(os.path.join(work, source))                       # the tool writes over nothing
                    words = ["tools/pack_build.py", command, "--recipe", recipe, "--out", source]
                    run(work, words + (["--display", os.path.abspath(display)] if display and command == "card" else []), log)
                    made.append(source)
    changed = []
    for path in made:
        data = read(os.path.join(work, path))
        if read(os.path.join(ROOT, path)) != data:
            with open(os.path.join(ROOT, path), "wb") as f:
                f.write(data)
            changed.append(path)
    page = functions_page(named, work)
    if read(os.path.join(ROOT, PAGE)) != page:
        with open(os.path.join(ROOT, PAGE), "wb") as f:
            f.write(page)
        changed.append(PAGE)
    return changed


def build(named, work, log):
    """{a pack file's name: its bytes}, built in one run."""
    profiles = [folder + "/profile.json" for folder in named["packs"]]
    said = {tuple(read_json(os.path.join(work, path))[key] for key in OF_THE_FIRMWARE) for path in profiles}
    if len(said) != 1:
        raise Problem("the packs' profiles are not all for one firmware file and one room")
    for path in profiles:
        run(work, ["tools/table_build.py", "--profile", path, "--check"], log)
    if read(os.path.join(ROOT, PAGE)) != functions_page(named, work):
        raise Problem("%s is not what the sources and %s give: run --make" % (PAGE, WORDS))
    recipes = []
    for folder in named["packs"]:
        recipes += ["--recipe", folder + "/recipe.json"]
    run(work, ["tools/pack_build.py", "build3"] + recipes + ["--profile", profiles[0], "--out-dir", "built"], log)
    names = [read_json(os.path.join(work, folder, "recipe.json"))["name"] + ".pack" for folder in named["packs"]]
    return {name: read(os.path.join(work, "built", name)) for name in names}


def version_reader(work):
    pack_build = tool_of(work, "pack_build")
    return lambda data: pack_build.parse(data)[0]["version"]


def hold(made, work, out, update):
    record = read_json(KEPT)
    kept, version = record["files"], version_reader(work)
    wrong, renewed = 0, {}
    for name, data in sorted(made.items()):
        with open(os.path.join(out, name), "wb") as f:
            f.write(data)
        facts = {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()}
        top = os.path.join(ROOT, name)
        before = read(top) if os.path.isfile(top) else None
        if kept.get(name) == facts and before == data:
            word = "the same"
        elif update:
            if before is not None and before != data and version(data) <= version(before):
                raise Problem("%s changed and its version is still %d: give it a new one in its recipe.json, then run --make" % (name, version(data)))
            renewed[name] = (facts, data)
            word = "updated"
        else:
            wrong += 1
            word = "DIFFERS"
        print("%-9s %-22s %8d bytes  %s" % (word, name, facts["bytes"], facts["sha256"]))
    for name, (facts, data) in renewed.items():
        kept[name] = facts
        with open(os.path.join(ROOT, name), "wb") as f:
            f.write(data)
    if renewed:
        with open(KEPT, "w", encoding="utf-8", newline="\n") as f:
            f.write(json.dumps(record, indent=1) + "\n")
    print("%d pack(s) built: %s" % (len(made), "%d differ from the kept ones" % wrong if wrong else
                                    "%d renewed, the others are the kept ones" % len(renewed) if renewed else "each is the kept one"))
    return 1 if wrong else 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--open-field", required=True, help="a checkout of open-field that has the commit open-field.json names")
    ap.add_argument("--open-xy", help="the toolchain's folder (default: open-xy in that checkout)")
    ap.add_argument("--out", default=os.path.join(ROOT, "out"), help="the folder to build in (new or empty; default: out beside this file)")
    ap.add_argument("--update", action="store_true", help="keep what was built: built.json and the files at the top written anew")
    ap.add_argument("--make", action="store_true", help="write the tables, cards and directories of pictures anew from what they are made from")
    ap.add_argument("--display", help="with --make: a copy of your unit's folder content/display, for the cards")
    args = ap.parse_args(argv)
    if args.make and args.update:
        print("error: --make first, then --update in a run of its own")
        return 2
    args.open_field = os.path.abspath(args.open_field)
    out = os.path.abspath(args.out)
    if os.path.isdir(out) and os.listdir(out):
        print("error: %s is not empty: this tool overwrites nothing there" % out)
        return 2
    try:
        os.makedirs(out, exist_ok=True)
        for folder, _ in held_icons(read_json(NAMED)):
            print("icon      %-22s in the page's form" % folder)
        named, work = stage(args, out)
        with open(os.path.join(out, "build.log"), "w", encoding="utf-8") as log:
            if args.make:
                changed = make(named, work, log, args.display)
                print("written anew: %s" % (", ".join(changed) or "nothing, each was as its source gives it"))
                return 0
            made = build(named, work, log)
        return hold(made, work, out, args.update)
    except (Problem, OSError, KeyError, ValueError) as problem:
        print("error: %s" % problem)
        return 2


if __name__ == "__main__":
    sys.exit(main())
