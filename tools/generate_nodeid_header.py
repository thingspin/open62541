#!/usr/bin/env python

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this 
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from __future__ import print_function
import sys
import platform
import getpass
import time
import argparse
from io import open

parser = argparse.ArgumentParser()
parser.add_argument('statuscodes', help='path/to/Opc.Ua.NodeIds.csv')
parser.add_argument('outfile', help='outfile w/o extension')
parser.add_argument('namespace', help='NS0')
args = parser.parse_args()

rows = []
with open(args.statuscodes, mode="rt") as f:
    lines = f.readlines()
    for l in lines:
        rows.append(tuple(l.strip().split(',')))

fh = open(args.outfile + ".h", "wt", encoding='utf8')
def printh(string):
    print(string, end=u'\n', file=fh)

#########################
# Print the header file #
#########################

printh(u'''/*---------------------------------------------------------
 * Autogenerated -- do not modify
 * Generated from {0} with script {1}
 *-------------------------------------------------------*/

#ifndef UA_NODEIDS_{2}_H_
#define UA_NODEIDS_{2}_H_

/**
 * Namespace Zero NodeIds
 * ----------------------
 * Numeric identifiers of standard-defined nodes in namespace zero. The
 * following definitions are autogenerated from the ``{0}`` file */
'''.format(args.statuscodes, sys.argv[0], args.namespace))

for row in rows:
    printh(u"#define UA_{namespace}ID_{name} {id} /* {description} */".format(namespace=args.namespace, name=row[0].upper(), id=row[1], description=row[2]))

printh(u'''#endif /* UA_NODEIDS_{0}_H_ */ '''.format(args.namespace))

fh.close()
