#!/usr/bin/python

import sys
import random
import os
from subprocess import check_output

path = os.path.dirname(os.path.realpath(__file__))
os.chdir(path)
print('Number of arguments:', len(sys.argv), 'arguments.')
print('Argument List:', str(sys.argv))

n_test = int(sys.argv[1]) if len(sys.argv) > 1 else 32 #number of types to generate

member_types = ['bool','char','int','double'] #types to generate in struct
member_types_nontrivial = ['std::vector<double>','std::unordered_map<int,long int>','std::string']
n_member_types = len(member_types)

random.seed(0)

n_members = 2 #number of member in each struct
max_size = 3 #maximum size of the array of type T
min_size = 1
content = ''
content += '#include<tuple>\n'
content += '#include<vector>\n'
content += '#include<string>\n'
content += '#include<unordered_map>\n'
#define struct
for i in range(n_test):
    content += 'struct T'+('%01d' % i)+' { '
    for j in range(n_members):
        t=random.randint(0,n_member_types-1)
        s=random.randint(min_size,max_size)
        content += member_types[t] + ' m'+str(j)+'['+str(s)+']{}; '
    t=random.randint(0,len(member_types_nontrivial))
    content += member_types_nontrivial[t] + ' m'+str(n_members+1)+'{}; ' if t<len(member_types_nontrivial) else ''
    content += '};\n'
#generate insert tuples
for j in range(n_test):
    content += 'using to_insert_'+str(j+1)+' = std::tuple<'
    list_T = []
    for i in range(j+1):
        list_T.append('T'+str(i)+'')
    content += ', '.join(list_T)
    content += '>;\n'
#generate get tuples
for j in range(n_test):
    content += 'using to_get_'+str(j+1)+' = std::tuple<'
    list_T = []
    list_j = list(range(n_test))
    list_j = random.sample(list_j,j+1)
    for i in range(j+1):
        list_T.append('T'+str(list_j[i-1])+'')
    content += ', '.join(list_T)
    content += '>;\n'
print(content)

containers = ['vecany','mapany','hc','ha','hcs','has','entt_ctx','andyg','entt_reg']
tests = ['1by1_insert','bulk_insert']

content += '#include "benchmark.h"\n'

with open('benchmark.h','r') as file:
    bench=file.read()
#content += bench

#generate insert
for i in range(1,9,1):#for each amount of types
    for test in tests:#for each test case
        for container in containers:#for each container
            if str(container+'_'+test) in bench:#if method is implemented
                content+='BENCHMARK_CAPTURE('+container+'_'+test+','+str(i)+', to_insert_'+str(i)+'{},to_get_'+str(i)+'{});\n'
#generate get n types out of n types max
tests = ['get']
for i in range(1,9,1):#for each amount of types
    for test in tests:#for each test case
        for container in containers:#for each container
            if str(container+'_'+test) in bench:#if method is implemented
                content+='BENCHMARK_CAPTURE('+container+'_'+test+','+str(i)+', to_insert_'+str(n_test)+'{},to_get_'+str(i)+'{});\n'

content+='BENCHMARK_MAIN();\n'
print(content)

f = open("benchmark.cpp", "w")
f.write(content)
f.close()
