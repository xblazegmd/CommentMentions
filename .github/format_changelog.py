import re

result = """> *"God"
> Bible 1:23 NKJV*

"""
titleCount = 0
with open("changelog.md", "r") as f:
    for line in f:
        if line.startswith("##"):
            titleCount += 1
            if titleCount > 1:
                break
            continue

        if line.startswith("#"):
            continue

        result += re.sub(r"<c[^>]+>([^<]*)</c>", r"\1", line)

print(result.strip(), end="")