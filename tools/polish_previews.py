#!/usr/bin/env python3
"""Own beta.3/candidate renderer comparisons; no reference-project images."""
import argparse
import json
from pathlib import Path
import shutil
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "docs/screenshots"
OLD = OUT / "beta3-before"
SCENES = ["yellow-long-trail", "green-long-trail", "trails-both", "trails-assist",
          "shared-same-d1-overview", "shared-opposite-d1-overview", "trails-zoom", "trails-assist-zoom"]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--baseline", type=Path)
    args = parser.parse_args()
    OLD.mkdir(exist_ok=True)
    if args.baseline:
        for name in SCENES:
            source = args.baseline / (name+".png")
            assert Image.open(source).size == (396,224)
            shutil.copyfile(source, OLD / source.name)
    sheet = Image.new("RGB", (816, len(SCENES)*248), "#d9dce0")
    draw = ImageDraw.Draw(sheet)
    for row, name in enumerate(SCENES):
        for col, (label, directory) in enumerate([("beta.3", OLD), ("candidate", OUT)]):
            x, y = col*408+6, row*248+3
            draw.text((x,y), label+" / "+name, fill="black")
            sheet.paste(Image.open(directory/(name+".png")), (x,y+18))
    sheet.save(OUT/"trail-before-after.png")
    names=[f"{actor}-step-{view}" for view in ["overview","zoom"] for actor in ["yellow","green"]]
    names += ["yellow-long-trail","green-long-trail","yellow-long-trail-zoom","green-long-trail-zoom",
              "trails-both","trails-zoom","trails-assist","trails-assist-zoom",
              "shared-same-d1-overview","shared-opposite-d1-overview","shared-same-d1-zoom","shared-opposite-d1-zoom"]
    sheet = Image.new("RGB",(816,8*248),"#d9dce0")
    draw=ImageDraw.Draw(sheet)
    for i,name in enumerate(names):
        x,y=(i%2)*408+6,(i//2)*248+3
        draw.text((x,y),name,fill="black")
        sheet.paste(Image.open(OUT/(name+".png")),(x,y+18))
    sheet.save(OUT/"trail-required-views.png")
    def rgb(value):
        return ((value>>11)*255/31, ((value>>5)&63)*255/63, (value&31)*255/31)
    def luminance(value):
        def linear(v):
            v/=255
            return v/12.92 if v<=0.04045 else ((v+0.055)/1.055)**2.4
        r,g,b=map(linear,rgb(value));return .2126*r+.7152*g+.0722*b
    def contrast(a,b):
        a,b=sorted([luminance(a),luminance(b)]);return round((b+.05)/(a+.05),3)
    colors={"old_yellow":0xac00,"old_green":0x0508,"trail_yellow":0xf5c0,"trail_green":0x5644,"assist":0x05da}
    backgrounds={"paper":0xef9e,"white_hole":0xffff,"yellow_camp":(31<<11)|(30<<6)|22,
                 "green_camp":(24<<11)|(30<<6)|26,"red_camp":(31<<11)|(26<<6)|26}
    (OUT/"trail-contrast.json").write_text(json.dumps({
        "rgb565":{k:f"0x{v:04x}" for k,v in colors.items()},
        "contrast_ratios":{k:{bg:contrast(v,b) for bg,b in backgrounds.items()} for k,v in colors.items()},
        "interpretation":"Color luminance only, not a text accessibility pass or LCD visibility measurement. Brighter lines trade luminance contrast for width and chroma; actual captures and hardware review are required.",
        "hardware":"HARDWARE TEST REQUIRED"},indent=2)+"\n")
    print("Own beta.3/candidate comparison, 16 required views and RGB565 contrast evidence generated")


if __name__ == "__main__":
    main()
