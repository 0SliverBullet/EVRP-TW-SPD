import os     
import math
def instancesGenerate():
    paths=["./data/evrptw_instances/small_instances/Cplex5er",
           "./data/evrptw_instances/small_instances/Cplex10er",
           "./data/evrptw_instances/small_instances/Cplex15er",
           "./data/evrptw_instances/large_instances"]
    for filePath in paths:
            fcases=os.listdir(filePath) 
            for pcase in fcases:  
                with open(filePath+"/"+pcase, "r") as file:
                    lines = file.readlines()

                new_lines = []
                data = lines[0].split()
                new_line = data[:4] + ["Delivery", "Pickup"] + data[5:]  
                line = '\t'.join(new_line) + '\n' 
                new_lines.append(line)

                for line in lines[1:]:
                    data = line.split()
                    if len(data) > 0:
                        if data[1] == 'd' or data[1] == 'f' or data[1] == 'c': 
                            demand = float(data[4])  
                            x = abs(float(data[2]))
                            y = abs(float(data[3]))
                            if x != 0  and y!= 0:
                                ρ = min(x/y, y/x)
                            else: 
                                ρ = 0.0 
                            # delivery = demand * ρ
                            delivery = float(math.floor(demand * ρ))
                            # the dataset used in the original paper of Akbay et al. (2022) automatically reserves 
                            # the integer part (directly truncating the decimal part).
                            pickup = demand - delivery
                            new_line = data[:4] + [str(delivery), str(pickup)] + data[5:] 
                            line = '\t'.join(new_line) + '\n'  
                    new_lines.append(line)
                new_filePath = filePath.replace("evrptw", "evrptwspd")
                with open(new_filePath+"/"+pcase, "w") as file:
                    file.writelines(new_lines)

if __name__=='__main__':
     instancesGenerate()
        


