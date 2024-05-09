#include<iostream>
#include<map>
#include<vector>
#include<fstream>
#include<string>
#include<set>
#include<algorithm>

using namespace std;

vector<string> inputLiterals, outputLiterals;
vector<vector<vector<set<int>>>> funcKernelsList;
vector<vector<set<int>>> bool_functions, kernels, auxKernels, funcCokernelsList;
vector<set<int>> co_kernels, auxCokernels, cubeRecord, auxFunc;
vector<vector<int>> auxIdxcubesInCoker;
map<string, int> conversionTable;
map<int, string> outConversionTable;
map<int, pair<int, int>> auxCubeToKernel;
vector<int> funcLiterals;
int inputSize, funcSize, originLiteralCount = 0;
string modelName = "";

bool containAndDelete(set<int>& copyCube, const set<int>& cubeB) {
    for (auto value : cubeB) {
        auto it = copyCube.find(value);
        if (it == copyCube.end()) return false;
        else copyCube.erase(it);
    }
    return true;
}
vector<set<int>> intersection(const vector<set<int>>& A, const vector<set<int>>& B) {
    vector<set<int>> result;
    int minSize = A.size();
    if (A.size() > B.size()) minSize = B.size();
    for (auto aCube : A) {
        for (auto bCube : B) {
            if (aCube == bCube) {
                result.push_back(aCube);
                break;
            }
        }
        if (result.size() == minSize) break;
    }
    return result;
}
vector<set<int>> multiplication(const vector<set<int>>& a, const vector<set<int>>& b) {
    vector<set<int>> result;
    set<int> combineCube;
    for (auto cubeA : a) {
        for (auto cubeB : b) {
            combineCube = cubeA;
            for (int value : cubeB) combineCube.insert(value);
            result.push_back(combineCube);
        }
    }
    return result;
}
vector<set<int>> difference(const vector<set<int>>& a, const vector<set<int>>& b) {
    vector<set<int>> result;
    set<int> remainIdx;
    for (int i = 0; i < a.size(); i++) remainIdx.insert(i);
    for (auto cubeB : b) {
        for (int i = 0; i < a.size(); i++) {
            if (cubeB == a[i]) {
                remainIdx.erase(i);
                break;
            }
        }
    }
    for (int idx : remainIdx) result.push_back(a[idx]);
    return result;
}
void division(const vector<set<int>>& dividend, const vector<set<int>>& divisor, vector<set<int>>& quotient, vector<set<int>>& remainder) {
    set<int> dividendL, copyCube;
    vector<set<int>> validCubes;
    int maxLiteral = 0;
    if (divisor.size() > dividend.size()) return;
    for (auto cube : dividend) {
        if (cube.size() > maxLiteral) maxLiteral = cube.size();
        for (auto value : cube) dividendL.insert(value);
    }
    for (auto cube : divisor) {
        if (cube.size() > maxLiteral) return;
        for (auto value : cube) {
            if (!dividendL.count(value)) return;
        }
    }
    for (int i = 0; i < divisor.size(); i++) {
        validCubes.clear();
        for (auto cubeA : dividend) {
            copyCube = cubeA;
            if (containAndDelete(copyCube, divisor[i])) validCubes.push_back(copyCube);
        }
        if (!validCubes.size()) return;
        if (i == 0) quotient = validCubes;
        else {
            quotient = intersection(quotient, validCubes);
            if (quotient.size() == 0) {
                remainder = dividend;
                return;
            }
        }
    }
    remainder = difference(dividend, multiplication(quotient, divisor));
}

void printAuxCubeToKernel() {
    cout << "Aux Cube to Kernel:\n";
    for (int i = 0; i < auxFunc.size(); i++) {
        pair<int, int> kerIdx = auxCubeToKernel[i];
        cout << "idx: " << i << ", kerIdx: (" << kerIdx.first << ", " << kerIdx.second << ")\n";
    }cout << endl;
}
void printLiteralsCount() {
    int count = 0;
    for (auto func : bool_functions) {
        for (auto cube : func) {
            count += cube.size();
        }
    }
    cout << "Optimized literal count: " << count << endl;
}
void printAuxKer_Coker() {
    cout << "Aux Kernels:\n";
    for (int i = 0; i < auxKernels.size(); i++) {
        cout << i << ": ";
        for (int j = 0; j < auxKernels[i].size(); j++) {
            if (j != 0) cout << " + ";
            cout << "(";
            for (auto it = auxKernels[i][j].begin(); it != auxKernels[i][j].end(); it++) {
                if (it != auxKernels[i][j].begin()) cout << " * ";
                cout << *it;
            }cout << ")";
        }cout << endl;
    }cout << endl;
    cout << "Aux Cokernels:\n";
    for (int i = 0; i < auxCokernels.size(); i++) {
        cout << i << ": (";
        for (auto it = auxCokernels[i].begin(); it != auxCokernels[i].end(); it++) {
            if (it != auxCokernels[i].begin()) cout << " * ";
            cout << *it;
        }cout << ")\n";
    }cout << endl;
    cout << "Aux idxCubes\n";
    for (int i = 0; i < auxIdxcubesInCoker.size(); i++) {
        cout << i << ": ";
        for (int j = 0; j < auxIdxcubesInCoker[i].size(); j++) {
            if (j != 0) cout << ", ";
            cout << "(" << auxCubeToKernel[auxIdxcubesInCoker[i][j]].first << ", " << auxCubeToKernel[auxIdxcubesInCoker[i][j]].second << ")";
        }cout << endl;
    }cout << endl;
}
void printCubeRecord() {
    cout << "Cube Record:\n";
    for (int i = 0; i < cubeRecord.size(); i++) {
        cout << i << ": ";
        for (auto it = cubeRecord[i].begin(); it != cubeRecord[i].end(); it++) {
            if (it != cubeRecord[i].begin()) cout << " * ";
            cout << inputLiterals[*it];
        }cout << endl;
    }cout << endl;
}
void printAuxFunc() {
    cout << "Aux Function:\n";
    for (int i = 0; i < auxFunc.size(); i++) {
        if (i != 0) cout << " + ";
        cout << "(";
        for (auto it = auxFunc[i].begin(); it != auxFunc[i].end(); it++) {
            if (it != auxFunc[i].begin()) cout << " * ";
            cout << *it;
        }cout << ")";
    }cout << "\n\n";
}
void printFunc(const vector<set<int>>& func) {
    for (int j = 0; j < func.size(); j++) {
        if (j != 0) cout << " + ";
        cout << "(";
        for (auto it = func[j].begin(); it != func[j].end(); it++) {
            if (it != func[j].begin()) cout << " * ";
            cout << inputLiterals[*it];
        } cout << ")";
    }cout << endl;
}
void printAllFuncs() {
    cout << "\nAll Bool Funcs:\n";
    for (int i = 0; i < bool_functions.size(); i++) {
        cout << "f" << i << " = " << outConversionTable[i] << endl;
        printFunc(bool_functions[i]);
    }cout << "\n";
}
void printInputLiterals() {
    cout << "\nInput Literals:\n";
    for (auto name : inputLiterals) cout << name << ", ";
    cout << endl;
}
void printOutputLiterals() {
    cout << "\nOutput Literals:\n";
    for (auto name : outputLiterals) cout << name << ", ";
    cout << endl;
}
void printCokernelsFunc() {
    set<int> cube;
    cout << "List of Function co-kernels:\n";
    for (int i = 0; i < funcCokernelsList.size(); i++) {
        if (!funcCokernelsList[i].size())continue;
        cout << "f" << i << ":\n";
        for (int j = 0; j < funcCokernelsList[i].size(); j++) {
            cube = funcCokernelsList[i][j];
            cout << j << ": (";
            for (auto it = cube.begin(); it != cube.end(); it++) {
                if (it != cube.begin()) cout << " * ";
                cout << inputLiterals[*it];
            }
            cout << ")\n";
        }cout << endl;
    }cout << endl;
}
void printKernelsFunc() {
    set<int> cube;
    cout << "List of Function Kernels:\n";
    for (int i = 0; i < funcKernelsList.size(); i++) {
        if (!funcKernelsList[i].size()) continue;
        cout << "f" << i << ":\n";
        for (int j = 0; j < funcKernelsList[i].size(); j++) {
            cout << j << ": ";
            for (int k = 0; k < funcKernelsList[i][j].size(); k++) {
                cube = funcKernelsList[i][j][k];
                if (k != 0) cout << " + ";
                cout << "(";
                for (auto it = cube.begin(); it != cube.end(); it++) {
                    if (it != cube.begin()) cout << " * ";
                    cout << inputLiterals[*it];
                }cout << ")";
            }cout << "\n";
        }cout << endl;
    }cout << endl;
}
void printKerCokerRem(const vector<set<int>>& kernel, const set<int>& co_kernel, const bool& secMode) {
    cout << "kernel: ";
    for (int i = 0; i < kernel.size(); i++) {
        if (i != 0) cout << " + ";
        cout << "(";
        for (auto it = kernel[i].begin(); it != kernel[i].end(); it++) {
            if (it != kernel[i].begin()) cout << " * ";
            if (!secMode) cout << inputLiterals[*it];
            else cout << *it;
        }cout << ")";
    }
    cout << "\n\nco-kernel: ";
    for (auto it = co_kernel.begin(); it != co_kernel.end(); it++) {
        if (it != co_kernel.begin()) cout << " * ";
        if (!secMode) cout << inputLiterals[*it];
        else cout << *it;
    }
}
void getWords(vector<string>& inputWords, const string& line, bool& keepGoing) {
    string word;
    int index = 0;
    while (index < line.size()) {
        if (line[index] == ' ' && word != "") {
            if (word == "\\") keepGoing = true;
            else {
                inputWords.push_back(word);
                word = "";
                keepGoing = false;
            }
        }
        else if (line[index] != ' ') word += line[index];
        index++;
    }
    if (word != "") {
        if (word == "\\") keepGoing = true;
        else {
            inputWords.push_back(word);
            keepGoing = false;
        }
    }
    //for (string str : inputWords) cout << str << ", ";
    //cout << endl;
}
void buildFunc(const vector<string>& literalNames, const vector<string>& inputWords) {
    set<int> cube;
    string literalName;
    for (int i = 0; i < inputWords[0].size(); i++) {
        literalName = literalNames[i];
        if (inputWords[0][i] == '0') literalName += "'";
        if (inputWords[0][i] != '-') {
            originLiteralCount++;
            cube.insert(conversionTable[literalName]);
        }
    }
    bool_functions[bool_functions.size() - 1].push_back(cube);
}
void CUBES(const vector<set<int>>& boolFunc, const int& literalIdx, vector<set<int>>& cubes, vector<int>& idxCubes, const vector<int>& cubesNo) {
    set<int> cube;
    for (int i = 0; i < boolFunc.size(); i++) {
        cube = boolFunc[i];
        auto it = cube.find(literalIdx);
        if (it != cube.end()) {
            cubes.push_back(cube);
            idxCubes.push_back(cubesNo[i]);
        }
    }
}
bool fromDiffFunc(const vector<set<int>>& kernel) {
    int type;
    for (auto value : kernel[0]) {
        if (value < 0) {
            type = value;
            break;
        }
    }
    for (int i = 1; i < kernel.size(); i++) {
        for (auto value : kernel[i]) {
            if (value < 0 && type != value) return true;
        }
    }
    return false;
}
void findKernels(const vector<set<int>>& boolFunc, const int start, const set<int>& pre_cokernel, const bool& secMode, const vector<int>& cubesNo) {
    vector<set<int>> cubes;
    vector<int> idxCubes;
    int literalIdx;
    map<int, int> literalCount;
    set<int> co_kernel;
    bool alreadyDone = false;
    for (int i = start; i < funcLiterals.size(); i++) {
        cubes.clear(), co_kernel.clear(); idxCubes.clear();
        literalIdx = funcLiterals[i];
        //if (!secMode) cout << "start: " << inputLiterals[literalIdx] << endl;
        //else cout << "start: " << literalIdx << endl;
        CUBES(boolFunc, literalIdx, cubes, idxCubes, cubesNo);
        if (cubes.size() > 1) {
            literalCount.clear();
            for (auto cube : cubes) {
                for (int idx : cube) {
                    auto it = literalCount.find(idx);
                    if (it == literalCount.end()) literalCount[idx] = 1;
                    else {
                        it->second++;
                        if (it->second == cubes.size() && it->first > -1) {
                            //if (!secMode) cout << "cnt: " << inputLiterals[it->first] << endl;
                            //else cout << "cnt: " << it->first << endl;
                            if (it->first < literalIdx) {
                                //cout << "Already Done\n";
                                alreadyDone = true;
                                break;
                            }
                            co_kernel.insert(it->first);
                            for (int cubeIdx = 0; cubeIdx < cubes.size() - 1; cubeIdx++)
                                cubes[cubeIdx].erase(it->first);
                        }
                    }
                }
                if (alreadyDone) break;
            }
            if (!alreadyDone) {
                for (auto Lidx : co_kernel) cubes[cubes.size() - 1].erase(Lidx);
                for (auto idx : pre_cokernel) co_kernel.insert(idx);
                //printKerCokerRem(cubes, co_kernel, secMode);
                if (!secMode) {
                    kernels.push_back(cubes);
                    co_kernels.push_back(co_kernel);
                    findKernels(cubes, i + 1, co_kernel, secMode, cubesNo);
                }
                else if (fromDiffFunc(cubes)) {
                    //cout << "From Different Func\n";
                    auxKernels.push_back(cubes);
                    auxCokernels.push_back(co_kernel);
                    auxIdxcubesInCoker.push_back(idxCubes);
                    findKernels(cubes, i + 1, co_kernel, secMode, idxCubes);
                }
            }
        }
    }
}
void findFuncLiterals(const vector<set<int>>& bool_func) {
    set<int> literals;
    funcLiterals.clear();
    for (auto cube : bool_func) {
        for (int idx : cube) {
            if (idx < 0) continue;
            auto it = literals.find(idx);
            if (it == literals.end()) {
                literals.insert(idx);
                funcLiterals.push_back(idx);
            }
        }
    }
    sort(funcLiterals.begin(), funcLiterals.end());
}
void kernelExtraction(bool& keepGoing, bool& multiKernel) {
    int count = 0;
    keepGoing = false;
    funcKernelsList.clear(); funcCokernelsList.clear();
    for (auto& bool_func : bool_functions) {
        co_kernels.clear(); kernels.clear();
        //cout << "Func:\n";
        //printFunc(bool_func);
        findFuncLiterals(bool_func);
        //cout << "\nFunc Literals:\n";
        //for (int idx : funcLiterals) cout << inputLiterals[idx] << ", ";
        //cout << endl << endl;
        findKernels(bool_func, 0, set<int>(), 0, vector<int>(bool_func.size()));
        funcCokernelsList.push_back(co_kernels);
        funcKernelsList.push_back(kernels);
        if (kernels.size()) {
            keepGoing = true;
            count++;
        }
    }
    if (count > 1) multiKernel = true;
    else multiKernel = false;
    //printCokernelsFunc();
    //printKernelsFunc();
}
int inCubeRecord(const set<int>& cube) {
    for (int i = 0; i < cubeRecord.size(); i++) {
        if (cubeRecord[i] == cube) return i;
    }
    return -1;
}
void buildAuxFunc() {
    set<int> newCube;
    int idx;
    cubeRecord.clear(); auxFunc.clear(); auxKernels.clear(); auxCokernels.clear(); auxIdxcubesInCoker.clear(); auxCubeToKernel.clear();
    for (int i = 0; i < funcKernelsList.size(); i++) {
        for (int j = 0; j < funcKernelsList[i].size(); j++) {
            newCube.clear();
            for (auto cube : funcKernelsList[i][j]) {
                idx = inCubeRecord(cube);
                if (idx == -1) {
                    cubeRecord.push_back(cube);
                    newCube.insert(cubeRecord.size() - 1);
                }
                else newCube.insert(idx);
            }
            newCube.insert(-(i + 1));
            auxCubeToKernel[auxFunc.size()] = make_pair(i, j);
            auxFunc.push_back(newCube);
        }
    }
    //printCubeRecord();
    //printAuxFunc();
    //printAuxCubeToKernel();
}
void kernelIntersection() {
    vector<int> cubesNo;
    funcLiterals.clear();
    for (int i = 0; i < cubeRecord.size(); i++)
        funcLiterals.push_back(i);
    for (int i = 0; i < auxFunc.size(); i++) cubesNo.push_back(i);
    findKernels(auxFunc, 0, set<int>(), 1, cubesNo);
    //printAuxKer_Coker();
}
void substitube(const int& targetFunIdx, const vector<set<int>>& replacedFunc, const set<int>& cokernel, const int& funcOutIdx) {
    vector<set<int>> newFunc, co_ker;
    int newLiteralIdx;
    string literalName;
    co_ker.push_back(cokernel);
    if (funcOutIdx != -1) {
        newLiteralIdx = inputLiterals.size() - 1;
        literalName = inputLiterals[inputLiterals.size() - 1];
    }
    else {
        newLiteralIdx = inputLiterals.size();
        literalName = "new" + to_string(inputLiterals.size() - inputSize);
        inputLiterals.push_back(literalName);
        conversionTable[literalName] = newLiteralIdx;
        bool_functions.push_back(replacedFunc);
        outConversionTable[bool_functions.size() - 1] = literalName;
    }
    //cout << "Func:" << outConversionTable[targetFunIdx] << endl;
    //printFunc(bool_functions[targetFunIdx]);
    //cout << "A * B:\n";
    //printFunc(multiplication(product, replacedFunc));
    //cout << "Remainder:\n";
    newFunc = difference(bool_functions[targetFunIdx], multiplication(co_ker, replacedFunc));
    //printFunc(newFunc);
    for (auto cube : co_ker) {
        cube.insert(newLiteralIdx);
        newFunc.push_back(cube);
    }
    bool_functions[targetFunIdx] = newFunc;
}
void singleKerExt(bool& keepGoing) {
    vector<set<int>> remainder, co_ker;
    int max, evaluate, kernelIdx, funcIdx, saveLiteralCount = 0;
    keepGoing = false;
    for (int i = 0; i < funcKernelsList.size(); i++) {
        funcIdx = i; max = 0; co_ker.clear(); kernelIdx = -1;
        for (int j = 0; j < funcKernelsList[i].size(); j++) {
            evaluate = (funcKernelsList[i][j].size() - 1) * funcCokernelsList[i][j].size() - 1;
            if (evaluate > max) {
                max = evaluate;
                kernelIdx = j;
            }
        }
        if (kernelIdx == -1) continue;
        saveLiteralCount += max;
        //cout << "f" << i << ", " << "ker:" << kernelIdx << " -> " << max << endl;
        substitube(i, funcKernelsList[i][kernelIdx], funcCokernelsList[i][kernelIdx], -1);
        keepGoing = true;
        //printAllFuncs();
    }
    //cout << "save :" << saveLiteralCount << endl;
}
bool mulKerExt() {
    int max = 0, gLiteralCount, literalCount, cubesLiteralCount, evaluate, cokerIdx = -1;
    map<int, int> matchedKers, tempKers;
    pair<int, int> originKerIdx;
    vector<set<int>> replaceFunc;
    for (int i = 0; i < auxCokernels.size(); i++) {
        gLiteralCount = 0; cubesLiteralCount = 0; tempKers.clear();
        for (int literalIdx : auxCokernels[i]) {
            gLiteralCount += cubeRecord[literalIdx].size();
        }
        for (int cubeIdx : auxIdxcubesInCoker[i]) {
            originKerIdx = auxCubeToKernel[cubeIdx];
            literalCount = funcCokernelsList[originKerIdx.first][originKerIdx.second].size();
            auto it = tempKers.find(originKerIdx.first);
            if (it != tempKers.end()) {
                //cout << "Duplicate: " << it->first << ", " << it->second << endl;
                if (funcCokernelsList[it->first][it->second].size() < literalCount) {
                    cubesLiteralCount += literalCount - funcCokernelsList[it->first][it->second].size();
                    it->second = originKerIdx.second;
                }
            }
            else {
                tempKers[originKerIdx.first] = originKerIdx.second;
                cubesLiteralCount += literalCount;
            }
        }
        evaluate = (tempKers.size() - 1) * gLiteralCount + (auxCokernels[i].size() - 1) * cubesLiteralCount - tempKers.size();
        //cout << "eva: " << evaluate << endl;
        if (evaluate > max) {
            max = evaluate;
            matchedKers = tempKers;
            cokerIdx = i;
        }
    }
    if (cokerIdx == -1) return false;
    for (int literalIdx : auxCokernels[cokerIdx]) {
        replaceFunc.push_back(cubeRecord[literalIdx]);
    }
    //cout << "mul max: " << max << endl;
    for (auto it = matchedKers.begin(); it != matchedKers.end(); it++) {
        if (it == matchedKers.begin()) substitube(it->first, replaceFunc, funcCokernelsList[it->first][it->second], -1);
        else substitube(it->first, replaceFunc, funcCokernelsList[it->first][it->second], bool_functions.size() - 1);
        //printAllFuncs();
    }
    return true;
}
void buildAuxFunc2() {
    auxFunc.clear(); auxKernels.clear(); auxCokernels.clear(); auxIdxcubesInCoker.clear(); auxCubeToKernel.clear();
    for (int i = 0; i < bool_functions.size(); i++) {
        int j = 0;
        for (auto cube : bool_functions[i]) {
            cube.insert(-(i + 1));
            auxCubeToKernel[auxFunc.size()] = make_pair(i, j);
            auxFunc.push_back(cube);
            j++;
        }
    }
}
void verticalKerExt() {
    vector<int> cubeNo;
    findFuncLiterals(auxFunc);
    //for (auto idx : funcLiterals) cout << "idx: " << idx << ", Name: " << inputLiterals[idx] << endl;
    for (int i = 0; i < auxFunc.size(); i++) cubeNo.push_back(i);
    findKernels(auxFunc, 0, set<int>(), 1, cubeNo);
}
void substituteCube(const set<int>& replacedCube,const int& coKerIdx) {
    vector<set<int>> newFunc;
    set<int> newCube;
    pair<int, int> cubePosition;
    newFunc.push_back(replacedCube);
    string literalName = "new" + to_string(inputLiterals.size() - inputSize);
    conversionTable[literalName] = inputLiterals.size();
    outConversionTable[bool_functions.size()] = literalName;
    bool_functions.push_back(newFunc);
    inputLiterals.push_back(literalName);
    for (int i = 0; i < auxKernels[coKerIdx].size(); i++) {
        newCube.clear();
        for (int value : auxKernels[coKerIdx][i]) {
            if (value < 0) continue;
            newCube.insert(value);
        }
        newCube.insert(inputLiterals.size() - 1);
        cubePosition = auxCubeToKernel[auxIdxcubesInCoker[coKerIdx][i]];
        bool_functions[cubePosition.first][cubePosition.second] = newCube;
    }
}
bool verticalKerEva() {
    int eva = 0, max = 0, cokerIdx = -1;
    for (int i = 0; i < auxCokernels.size(); i++) {
        eva = auxKernels[i].size() * (auxCokernels[i].size() - 1) - auxCokernels[i].size();
        if (eva > max) {
            cokerIdx = i;
            max = eva;
        }
    }
    if (cokerIdx == -1) return false;
    //cout << "save: " << max << endl;
    substituteCube(auxCokernels[cokerIdx], cokerIdx);
    return true;
}
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
                //cout << "aux1 done\n";
                //cout << "size: " << auxFunc.size() << endl;
                kernelIntersection();
                //cout << "R_ker done\n";
                if (auxCokernels.size() == 0) method = 3;
                else if (!mulKerExt()) method = 3;
                //else printLiteralsCount(); cout << endl;
            }
            if (method == 3) {
                //cout << "single kernel:\n";
                for (auto func : bool_functions) size += func.size();
                //cout << "size: " << size << endl;
                singleKerExt(keepGoing);
                //cout << "R_ker done\n";
                //printLiteralsCount(); cout << endl;
            }
        }
    }

    //printAllFuncs();
    cout << "Original literal count: " << originLiteralCount << endl;
    printLiteralsCount();
}
void writeOutputFile() {
    set<int> literals;
    vector<int> funcOutLiterals;
    fstream file("./out.blif", ios::out);
    file << ".model ";
    if (modelName == "") file << "sample\n";
    else file << modelName << endl;
    file << ".intput";
    for (int i = 1; i < inputSize; i+=2) {
        file << " " << inputLiterals[i];
        if (i == inputSize - 1) file << endl;
    }
    file << ".output";
    for (int i = 0; i < funcSize; i++) {
        file << " " << outputLiterals[i];
        if (i == funcSize - 1) file << endl;
    }
    for (int i = 0; i < bool_functions.size(); i++) {
        literals.clear(); funcOutLiterals.clear();
        for (auto cube : bool_functions[i]) {
            for (int value : cube) {
                if (value < inputSize && value % 2 == 0) value += 1;
                auto it = literals.find(value);
                if (it == literals.end()) {
                    literals.insert(value);
                    funcOutLiterals.push_back(value);
                }
            }
        }
        sort(funcOutLiterals.begin(), funcOutLiterals.end());
        file << ".name ";
        for (int idx : funcOutLiterals) file << inputLiterals[idx] << " ";
        file << outConversionTable[i] << endl;
        for (auto cube: bool_functions[i]) {
            for (int k = 0; k < funcOutLiterals.size(); k++) {
                int key = funcOutLiterals[k];
                if (key < inputSize) {
                    auto it1 = cube.find(key);
                    auto it2 = cube.find(key - 1);
                    if (it1 != cube.end()) file << "1";
                    else if (it2 != cube.end()) file << "0";
                    else file << "-";
                }
                else {
                    auto it = cube.find(key);
                    if (it != cube.end()) file << "1";
                    else file << "-";
                }
            }
            file << " 1\n";
        }
    }
    file << ".end";
}

int main(int argc, char** argv)
{
    if (argc != 2) cout << "number of argument is wrong\n";
    else {
        bool isFunc = false, keepGoing = false;
        vector<string> inputWords;
        fstream file(argv[1], ios::in);
        if (!file) cout << "file can't open\n";
        else {
            string line;
            vector<string> inputWords, literalNames;
            while (getline(file, line)) {
                inputWords.clear();
                getWords(inputWords, line, keepGoing);
                if (inputWords.size() == 0) continue;
                if (inputWords[0] == ".end") break;
                else if (inputWords[0] == ".model") {
                    modelName = inputWords[1];
                }
                else if (inputWords[0] == ".inputs") {
                    while (keepGoing && getline(file, line))
                        getWords(inputWords, line, keepGoing);
                    for (int i = 1; i < inputWords.size(); i++) {
                        inputLiterals.push_back(inputWords[i] + "'");
                        conversionTable[inputWords[i] + "'"] = inputLiterals.size() - 1;
                        inputLiterals.push_back(inputWords[i]);
                        conversionTable[inputWords[i]] = inputLiterals.size() - 1;
                    }
                    inputSize = inputLiterals.size();
                    isFunc = false;
                }
                else if (inputWords[0] == ".outputs") {
                    while (keepGoing && getline(file, line))
                        getWords(inputWords, line, keepGoing);
                    for (int i = 1; i < inputWords.size(); i++)
                        outputLiterals.push_back(inputWords[i]);
                    isFunc = false;
                }
                else if (inputWords[0] == ".names" || isFunc) {
                    while (keepGoing && getline(file, line))
                        getWords(inputWords, line, keepGoing);
                    if (inputWords[0] == ".names") {
                        literalNames.clear();
                        for (int i = 1; i < inputWords.size(); i++)
                            literalNames.push_back(inputWords[i]);
                        bool_functions.push_back(vector<set<int>>());
                        outConversionTable[bool_functions.size() - 1] = literalNames[literalNames.size() - 1];
                        isFunc = true;
                    }
                    else buildFunc(literalNames, inputWords);
                }
            }
            funcSize = bool_functions.size();
            //printInputLiterals();
            //printOutputLiterals();
            //printAllFuncs();
            //cout << "\n\nOriginal literal count: " << originLiteralCount << endl;
            optimization();
            writeOutputFile();
        }
    }
}

/*
void optimization() {
    bool keepGoing = true, multi = true;
    int method = 1, size = 0;
    while (keepGoing) {
        size = 0;
        if (method == 1) {
            cout << "vertical:\n";
            buildAuxFunc2();
            cout << "aux2 done\n";
            cout << "size: " << auxFunc.size() << endl;
            verticalKerExt();
            cout << "R_ker done\n";
            if (auxCokernels.size() == 0) method = 2;
            else if (!verticalKerEva()) method = 2;
            else printLiteralsCount(); cout << endl;
            //printAllFuncs();
        }
        if (method > 1) {
            kernelExtraction(keepGoing, multi);
            if (method == 2) {
                if (!multi) method = 3;
                else {
                    cout << "multi kernel:\n";
                    buildAuxFunc();
                    cout << "aux1 done\n";
                    cout << "size: " << auxFunc.size() << endl;
                    kernelIntersection();
                    cout << "R_ker done\n";
                    if (auxCokernels.size() == 0) method = 3;
                    else if (!mulKerExt()) method = 3;
                    else printLiteralsCount(); cout << endl;
                }
            }
            if (method == 3) {
                cout << "single kernel:\n";
                for (auto func : bool_functions) size += func.size();
                cout << "size: " << size << endl;
                singleKerExt(keepGoing);
                cout << "R_ker done\n";
                printLiteralsCount(); cout << endl;
            }
        }
    }

    printAllFuncs();
    cout << "\n\nOriginal literal count: " << originLiteralCount << endl;
    printLiteralsCount();
}
*/