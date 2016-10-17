f = open('scalability.txt' , 'r')
sum = 0
count = 0.0
data = []
while True :
    tmp = f.readline()
    if tmp=='':
        break
    tmpdict = tmp.split(' ')
    sum += int(tmpdict[1])
    data.append(int(tmpdict[1]))
    count += 1.0
average = (sum / count)
index = 0
variance = 0.0
print("Average: "+str(average)+" tasks")

while index!=count:
    variance += (data[index] - average) ** 2
    index += 1

print("Variance: " + str(variance))