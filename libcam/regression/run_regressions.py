#!/usr/bin/python3

import os, subprocess

tests = """
test0
test1
test2
test3
test4
test5
""".split()

def removefile(f):
  try:
    os.remove(f)
  except OSError:
    pass

for test in tests:
  print('running', test)
  removefile(test+'/dependence_pairs.txt')
  subprocess.call(['cam', '-p'], cwd=test)
  if subprocess.call(['diff', 'dependence_pairs.txt', 'oracle.txt'], cwd=test):
    print('  error')

