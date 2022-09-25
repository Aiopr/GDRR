import functools

#filename=input()
filename='test.out'
#filename='test1.txt'
with open(filename) as f:
    datas=f.read().split()
datas=[int(x) for x in datas]
#print(datas)

binwidth=datas[0]
binheight=datas[1]
binnum=datas[2]
totalarea=binwidth*binheight*binnum

recs=[]
leftover=0
for i in range(3,len(datas),6):
    #print(i)
    if(datas[i]==-1):
        leftover=leftover+datas[i+2]*datas[i+3]
    recs.append([datas[i],datas[i+1],datas[i+2],datas[i+3],datas[i+4],binheight-datas[i+3]-datas[i+5]])
#print(recs)

def recs_cmp(rec1,rec2):
    if(rec1[1]<rec2[1]):
        return -1
    elif(rec1[1]>rec2[1]):
        return 1
    if(rec1[5]<rec2[5]):
        return -1
    elif(rec1[5]>rec2[5]):
        return 1
    if(rec1[4]<rec2[4]):
        return -1
    elif(rec1[4]>rec2[4]):
        return 1
    return 1

recs.sort(key= functools.cmp_to_key(recs_cmp))
#print(recs)

def get_font_size(a,b):
    return min(max(a,b)*0.005,min(a,b)*0.02)

Dicxy=binwidth*0.002
binbox='<div style="width:1000px;"><svg height="100%" viewBox="'+str(-1*Dicxy)+' '+str(-1*Dicxy)+' '+str(binwidth+2*Dicxy)+' '+str(binheight+2*Dicxy)+'" width="100%" xmlns="http://www.w3.org/2000/svg">'+'\n'+'<g>'+'\n'

Usage=(totalarea-leftover)/totalarea
outfile=open(filename+'.html','w+')
outfile.write('<!DOCTYPE html><html style="font-family:Arial"><head><title>Solution large_example</title></head><body><h1>Solution</h1><h2>Statistics</h2><table><tr><th style="text-align:left">Usage</th><td>'+str(100*Usage)+'%</td></tr><tr><th style="text-align:left">Part area included</th><td>100.000%</td></tr><tr><th style="text-align:left"># Objects used</th><td>'+str(binnum)+'</td></tr><tr><th style="text-align:left">Material cost</th><td>'+str(totalarea)+'</td></tr></table>'+'\n')
outfile.write('<h2>Cutting Patterns</h2>'+'\n')
currec=-1
for rec in recs:
    if(currec<rec[1]):
        currec=currec+1
        if(currec>0):
            outfile.write('</g></svg></div>'+'\n')
        outfile.write('<h3>Pattern '+str(currec)+'</h3>'+'\n')
        outfile.write(binbox)
    outfile.write('<g>\n')
    if(rec[0]>0):
        outfile.write('<rect fill="#BFBFBF" height="'+str(rec[3])+'" stroke="black" stroke-width="'+str(Dicxy)+'" width="'+str(rec[2])+'" x="'+str(rec[4])+'" y="'+str(rec[5])+'"/>\n')
        if(rec[2]>=rec[3]):
            outfile.write('<text dominant-baseline="middle" fill="black" font-size="'+str(get_font_size(rec[2],rec[3]))+'em" text-anchor="middle" x="'+str(rec[2]/2+rec[4])+'" y="'+str(rec[3]/2+rec[5])+'">\n')
        else:
            outfile.write('<text dominant-baseline="middle" fill="black" font-size="'+str(get_font_size(rec[2],rec[3]))+'em" text-anchor="middle" transform="rotate(-90 '+str(rec[2]/2+rec[4])+' '+str(rec[3]/2+rec[5])+')" x="'+str(rec[2]/2+rec[4])+'" y="'+str(rec[3]/2+rec[5])+'">\n')
        outfile.write(str(rec[0])+': ['+str(rec[2])+'x'+str(rec[3])+']\n')
        outfile.write('</text>\n')
    else:
        outfile.write('<rect fill="#A9D18E" height="'+str(rec[3])+'" stroke="black" stroke-width="'+str(Dicxy)+'" width="'+str(rec[2])+'" x="'+str(rec[4])+'" y="'+str(rec[5])+'"/>\n')
    outfile.write('</g>\n')
outfile.write('</g></svg></div>'+'\n')
outfile.write('</svg></div></body></html>')
