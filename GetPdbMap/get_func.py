import idaapi
import idc
import os
   
def stdout_to_file(output_file_name, output_dir=None):

    if not output_dir:
        output_dir = os.path.dirname(os.path.realpath(__file__))

    output_file_path = os.path.join(output_dir, output_file_name)
    orig_stdout = sys.stdout
    f = file(output_file_path, "w")

    sys.stdout = f

    return f, orig_stdout
    
def main(args):
    i = 0
    f, orig_stdout = stdout_to_file("output.txt")
    if idc.ARGV:
        for i, arg in enumerate(idc.ARGV):
            print "[*] arg[{}]: {}".format(i, arg)
            
    if i < 1:
        print "miss param"
        return
        
    pefile = idc.ARGV[1]
    print "pefile: %s" % (pefile)
    tmpfile,tmptype = os.path.splitext(pefile)
    pdbmappath = tmpfile + '.pdbmap'
    
    print "pdbmappath: %s" % (pdbmappath)
    base = idaapi.get_imagebase()
    with open(pdbmappath,'w+') as file_handle:
        for funcAddr in Functions():
            funcName = GetFunctionName(funcAddr)
            end = idc.GetFunctionAttr(funcAddr, FUNCATTR_END)
            size = end - funcAddr
            
            logstr = funcName + ' '+ str(size) + ' 5 ' + '0x{:X}'.format(funcAddr-base) + '\n'
            print "Function %s is at 0x%x, size: %d" % (funcName, funcAddr-base,size)
            
            file_handle.write(logstr)
        file_handle.close

        
if __name__=="__main__":
    idaapi.autoWait()
    main(sys.argv)
    idc.Exit(0)