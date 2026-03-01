import os
import subprocess


DRIVER = 'build/debug/compiler.exe'

def compare(out, path):

    try:
        with open(path, 'r') as fd:
            lines = fd.readlines()
    except FileNotFoundError:
        print('{} not found'.format(path))
        with open(path + '_', 'wb') as fd:
            fd.write(out)
        return False

    out = out.decode('utf-8')
    out = out.splitlines()
    for l in lines:
        got = out[0].strip()
        expect = l.strip()
        if got != expect:
            print('got:{} expected:{}'.format(got, expect))
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

    return compare(out, path + '.expect')

def main():
    root = 'tests'
    tests = os.listdir(root)
    results = []
    for test in filter(lambda f: os.path.splitext(f)[1] == '.c', tests):
        r = run_test(os.path.join(root, test))
        results += [ (test, r) ]
    for r in results:
        print('{:<20} {}'.format(r[0], 'Pass' if r[1] else 'Fail'))

main()
