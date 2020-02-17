
if __name__ == '__main__':
    try:
        mode = input("Mode: t or b:")
        f1 = input("File one:")
        f2 = input("File two:")
        if mode == 'b':
            with open(f1, "rb") as r1:
                c1 = list(r1.read())
                # print(c1)
                with open(f2, "rb") as r2:
                    c2 = list(r2.read())

                    for i in range(len(c1)):
                        ch1 = c1[i]
                        ch2 = c2[i]
                        if ch1 != ch2:
                            print("Not equal at {}. File 1: {}, File 2: {}"
                                  .format(i, ch1, ch2))
        else:
            with open(f1, "r") as r1:
                c1 = r1.readlines()
                with open(f2, "r") as r2:
                    c2 = r2.readlines()

                    for i in range(len(c1)):
                        line1 = c1[i].split(" ")
                        line2 = c2[i].split(" ")
                        if len(line1) != len(line2):
                            print("Length diff at line {}".format(i))
                        for j in range(min(len(line1), len(line2))):
                            ch1 = line1[j]
                            ch2 = line2[j]
                            if ch1 != ch2:
                                print("Not equal at line {}, num {}. File 1: {}, File 2: {}"
                                      .format(i, j, ch1, ch2))
                                print("Whole line in file 1: {}".format(line1))
    except Exception as e:
        print(e)

    input("exit..")
