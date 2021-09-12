import time
import datetime
import sys
import getopt
import os

 
a_time = None
s_second = None
s_microsecone = None
abs_time = 0.0
inputfile = None
outputfile = None
 
def usage():
    print('''Help Information:
             -h, --help:        Show help information
             -i, --inputfile:   input file  to parse
             -o, --outputfile:  output fiel parsed
                        ''')
 
 
def calc_delta(stream):
    global s_second
    global s_microsecond
    global a_time
    global outfile
    begin_index = None
    end_index = None
    delta_second = 0
    delta_mircosecond = 0
    delta_time = 0
    d_time = None
    new_line = None
    result = None
    if a_time ==None:
        print("Can't convert to android time")
        exit(-1)
    for line in stream:
        if line:
            try:
                result = line.find('UTC)')
                if result >=1:
                    get_agin_time(line)
                begin_index =  line.index('>[')
                end_index = line[begin_index+1:].index(']')+begin_index+1
                time_string = line[begin_index + 2 :end_index].strip()
                [d_second,d_microsecond] = time_string.split('.')
                delta_second = int(int(d_second) - int(s_second)) + 28800
                #delta_second = int(int(d_second) - int(s_second))
                delta_microsecond = int(int(d_microsecond)-int(s_microsecond))
                delta_time = datetime.timedelta(seconds=delta_second,microseconds=delta_microsecond)
                d_time = a_time + delta_time
                new_line = d_time.strftime("%Y-%m-%d %H:%M:%S.%f")+' ' + line
                outputfile.write(new_line)
            except:
                outputfile.write(line)
 
 
def get_agin_time(han_test):
    global s_second
    global s_microsecond
    global a_time
    a_time_op = None
    begin_index = None
    end_index = None
    data_start = None
    data_end = None
    result = None
    a_time_op = han_test.find('UTC)')
    if a_time_op>=1:
        begin_index =  han_test.index('>[')
        end_index = han_test[begin_index+1:].index(']')+begin_index+1
        data_start = han_test[a_time_op-45:].index('(') + a_time_op-45
        data_end = han_test[data_start+1:].index(')')+data_start+1
        result = data_end - data_start - 5
        if result > 26:
            date_string = han_test[data_start+1:data_end-7].strip()
        else:
            date_string = han_test[data_start+1:data_end-4].strip()
        abs_time = han_test[begin_index + 2 :end_index].strip()
        [s_second,s_microsecond] = abs_time.split('.')
        a_time = datetime.datetime.strptime(date_string, "%Y-%m-%d %H:%M:%S.%f")
  
def get_atime(stream):
    global s_second
    global s_microsecond
    global a_time
    a_time_op = None
    begin_index = None
    end_index = None
    result = None
    for line in stream:
        if line:
            a_time_op = line.find('UTC)')
            if a_time_op>=1:
                begin_index =  line.index('>[')
                end_index = line[begin_index+1:].index(']')+begin_index+1
                data_start = line[a_time_op-45:].index('(') + a_time_op-45
                data_end = line[data_start+1:].index(')')+data_start+1
                result = data_end - data_start - 5
                if result > 26:
                    date_string = line[data_start+1:data_end-7].strip()
                else:
                    date_string = line[data_start+1:data_end-4].strip()
                abs_time = line[begin_index + 2 :end_index].strip()
                [s_second,s_microsecond] = abs_time.split('.')
                a_time = datetime.datetime.strptime(date_string, "%Y-%m-%d %H:%M:%S.%f")
                break
 
def main(argv):
    global inputfile
    global outputfile
    inputpath = None
    outputpath = None
    try:
        opts, args = getopt.getopt(argv,"hi:o:",["help","inputfile=","outputfile="])
    except getopt.GetoptError:
        usage()
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            usage()
            sys.exit()
        if opt in ("-i", "--inputfile"):
            inputpath = arg
        if opt in ("-o", "outputfile"):
            outputpath = arg
    if inputpath == None:
        usage()
        sys.exit()
    if outputpath == None:
        outputpath = os.getcwd()+"/out.txt"
 
    inputfile = open(inputpath, 'r')
    outputfile = open(outputpath, 'w')
    get_atime(inputfile)
    inputfile.seek(0)
    calc_delta(inputfile)
    inputfile.close()
    outputfile.close()
if __name__ == "__main__":
    main(sys.argv[1:])
