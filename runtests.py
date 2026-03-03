import os
import subprocess


def findDriver():
    winPath = 'build/debug/compiler.exe'
    lnxPath = './compiler'
    if os.path.exists(winPath):
        return winPath
    if os.path.exists(lnxPath):
        return lnxPath
    return 'unknown'

DRIVER = findDriver()

def compare(out, path):

    try:
        with open(path, 'r') as fd:
            lines = fd.readlines()
    except FileNotFoundError:
        print('{} not found'.format(path))
        with open(path + '', 'wb') as fd:
            fd.write(out)
        return False

    out = out.decode('utf-8')
    out = out.splitlines()

    for l in lines:
        if not out:
            return False
        got = out[0].strip()
        expect = l.strip()
        if got != expect:
            print('{}:\n  got:{} expected:{}'.format(path, got, expect))
            return False
        out = out[1:]
    return True


def run_test(path):
    try:
        proc = subprocess.Popen(
            [DRIVER, path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)

        out, err = proc.communicate()
        ret = proc.returncode

    except OSError as e:
        print('failed to execute {} {}'.format(DRIVER, path))
        print(e)
        return

    return compare(out, path + '.expect')


def main():
    root = 'tests'
    tests = os.listdir(root)
    results = []
    
    for (dirpath, dirs, files) in os.walk('tests'):
        for test in filter(lambda f: os.path.splitext(f)[1] == '.c', files):
            test_path = os.path.join(dirpath, test)
            r = run_test(test_path)
            results += [ (test_path, r) ]
    for r in results:
        print('{:<40} {}'.format(r[0], 'Pass' if r[1] else 'Fail'))

main()
