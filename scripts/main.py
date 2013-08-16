#!/usr/bin/env python

import sys
import transform2

# #######################################################
# Sample python script that invokes the transform script
# #######################################################

def main():

    # calling transform script
    bitcodefile = sys.argv[1]
    configfile = sys.argv[2]
    result = transform2.transform(bitcodefile, configfile)
    print "Return value: " + str(result)
    
if __name__ == "__main__":
    main()
