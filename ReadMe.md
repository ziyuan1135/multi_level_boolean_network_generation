# 介紹
如今，大多數的combinational circuits都是以multi-level Boolean network的形式實現的。邏輯閘的fine granularity為電路設計師提供了功率消耗、面積與效能最佳化的多種選擇。
然而，**multi-level logic synthesis**的設計彈性帶來了**很大的運算複雜度**。目前只制定了**少數受限的方法**來進行multi-level Boolean **optimization**。
在這個專案中，我們實現一種基於**啟發式的algebraic multi-level Boolean network generation**，以實現**最少literal count**，進而降低面積。通過應用algebraic division, decomposition, kernel extraction, and substitution等操作，輸出BLIF格式的multi-level Boolean network。

# 如何使用
## 步驟
```
1. 輸入 'make' 指令進行編譯
2. 輸入格式: ./multilevel ./BLIF檔位置
e.g. ./multilevel ./example.blif
```
## 輸入: two-level BLIF檔
### sample.blif
```
.model sample
.inputs a b c d e f g h i j k l m n
.outputs o p
.names a b c d e g h i j k o
01111---1- 1
01011--11- 1
010111--0- 1
11111---11 1
11011-111- 1
.names c d e f g h l m n p
1111---1- 1
-001--0-0 1
101111--0 1
.end
```
### 片段說明
```
.names a b c d e g h i j k o
01111---1- 1
01011--11- 1
```
1. 輸出: o
2. 輸入: a, b, c, d, e, g, h, i, j, k
4. o = a'bcdej + a'bc'deij
## 輸出: BLIF檔與literal count
### run time example
```
./multilevel sample.blif
Original literal count: 52
Optimized literal count: 34
```
### out.blif
```
.model sample
.inputs a b c d e f g h i j k l m n
.outputs o p
.names b d e v o
1111 1
.names f y p
11 1
.names a c g j w v
---11 1
0010- 1
.names a c h i k x w
-1--1- 1
0----1 1
-011-- 1
.names c i x
1- 1
-1 1
.names c d e l n z y
1-1--1 1
-0000- 1
.names d g h m n z
1--1- 1
011-0 1
.end
```

# 理論說明
## Example of General Network
![image](https://hackmd.io/_uploads/BJ4lPD9fC.png)
## Decomposition
* Introduce new variables/blocks into the network
* Example:
\- v = a'd + bd +c'd +ae'
\- j = a' + b + c'; v = jd + ae';

![image](https://hackmd.io/_uploads/B1R1uP5GA.png)

## Extraction
* Find a common sub-expression of two(or more) expression
\- Extract new sub-expression as new function
\- Introduce new block into the circuit
* Example
\- p = ce + de; t = ac + ad + bc + bd + e;
\- p = ( c + d ) e; t = ( c + d ) ( a + b ) + e;
\- k = c + d; p = ke; t = ka + kb + e
![image](https://hackmd.io/_uploads/SJpc_PqM0.png)

## Algebraic Division
![image](https://hackmd.io/_uploads/By3iFDqfR.png)
### Example
![image](https://hackmd.io/_uploads/Bk3k5vqzA.png)
### Algorithm for Division
![image](https://hackmd.io/_uploads/rJ2-qwqz0.png)

## Definitions
* Cube-free expression
\- Expression that cannot be factored by a cube
\- Example:
\* a + bc is cube free
\* abd and ab + ac are not
* Kernel of an expression
\- **Cube-free  quotient** of the expression **divided by a cube** (The cube is called co-kernel)
* Kernel set of an expression f is denoted by K(f)
### Example
![image](https://hackmd.io/_uploads/Hkay2v9MA.png)

## Kernel method
### Recursive Kernel Computation
![image](https://hackmd.io/_uploads/r1IGTvqfR.png)
#### Example
![image](https://hackmd.io/_uploads/ryUNpw5GC.png)

### Single cube extraction
* Extract one cube from two (or more) subexpressions [Brayton]
* Form an auxiliary expression, which is the union (sum) of all local expression
* Find the largest co-kernel
\- Corresponding kernel must belong to two (or more) different expressions
\- Use additional variables to tag the expressions
* Extract chosen co-kernel
#### Example
![image](https://hackmd.io/_uploads/Byr7CPcMC.png)

### Multiple-cube Extraction
* We need a cube/kernel matrix
\- Relabel cubes by new variables
\- Kernels are now cubes in these new variables
* Equivalently, find a co-kernel of the auxiliary expression that is the sum of the relabeled expre
#### Example
![image](https://hackmd.io/_uploads/HkjcCP9fA.png)

# 程式流程
```c!
void optimization() {
    bool keepGoing = true, multi = true;
    int method = 1, size = 0;
    while (keepGoing) {
        size = 0;
        if (method == 1) {
            //cout << "vertical:\n";
            buildAuxFunc2();
            //cout << "aux2 done\n";
            //cout << "size: " << auxFunc.size() << endl;
            verticalKerExt();
            //cout << "R_ker done\n";
            if (auxCokernels.size() == 0) method = 2;
            else if (!verticalKerEva()) method = 2;
            //else printLiteralsCount(); cout << endl;
            //printAllFuncs();
        }
        if (method > 1) {
            kernelExtraction(keepGoing, multi);
            if (!multi) method = 3;
            else {
                //cout << "multi kernel:\n";
                buildAuxFunc();
                // cout << "aux1 done\n";
                // cout << "size: " << auxFunc.size() << endl;
                kernelIntersection();
                //cout << "R_ker done\n";
                if (auxCokernels.size() == 0) method = 3;
                else if (!mulKerExt()) method = 3;
                else printLiteralsCount(); cout << endl;
            }
            if (method == 3) {
                //cout << "single kernel:\n";
                for (auto func : bool_functions) size += func.size();
                //cout << "size: " << size << endl;
                singleKerExt(keepGoing);
                //cout << "R_ker done\n";
                printLiteralsCount(); cout << endl;
            }
        }
    }

    //printAllFuncs();
    cout << "Original literal count: " << originLiteralCount << endl;
    printLiteralsCount();
}
```
1. 首先，我們利用Single cube extraction將所有的expressions合併成一個，並利用verticalKerExt()提出最大的co-kernel，其中co-kernel屬於兩個以上的expressions。
若找不到co-kernel，則跳到(2.)
2. 利用kernelExtraction()將每個expression的kernel與co-kernel找出來，若有兩個以上的expressions具有kernel，則跳到(3.)；否則，跳到(4.)
3. 利用Multiple-cube Extraction方法尋找最大的sub-kernel，其中sub-kernel屬於兩個以上的expressions。若找不到，則跳到(4.)
4. 對每個expression進行kernel或co-kernel的替換
5. 重複步驟(1.) ~ (4.)直到沒有kernel或替換後不會減少literal count
