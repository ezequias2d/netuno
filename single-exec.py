import subprocess, glob, os

testdir = "./tests"
runner = "./bin/ntc"

def test(file):
    try:
        proc = subprocess.run([runner, file], capture_output=True, text=True, timeout=2)
        outs = proc.stdout
        errs = proc.stderr
    except subprocess.TimeoutExpired as timeErr:
        outs = timeErr.stdout
        errs = timeErr.stderr
        return False
    if proc.returncode != 0:
        return False

    with open(file + ".expected") as f:
        expect = f.read()

    if expect != outs:
        return False

    return True


total = 0
ok = 0
fail = 0
for file in sorted(glob.glob(testdir + "/*.nt")):
    basename = os.path.basename(file)
    total += 1
    if test(file):
        print("ok\t" + basename)
        ok += 1
    else:
        print("fail\t" + basename)
        fail += 1

print(f"{ok}/{total} (pass: {ok}, fail: {fail})")

