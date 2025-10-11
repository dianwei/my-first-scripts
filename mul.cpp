#include <iostream> 
#include <string>   // 用于 std::stod 和 std::string
#include <stdexcept> // 用于捕获 std::stod 可能抛出的异常 (std::invalid_argument, std::out_of_range)
#include <vector>

std::string bigbigmul (const std::string &num1, const std::string &num2);
int main(int argc, char* argv[]) {
    // 检查命令行参数数量
    if (argc < 3 || argc > 4) {
        // 如果参数数量不对，向标准错误流输出用法提示，并返回非零值表示错误。
        std::cerr << "用法: " << argv[0] << " （参数）<数字1> <数字2>" << std::endl;
        for(int i=0;i<argc;i++){
            std::cout << argv[i] << std::endl;
        }
        return 1; 
    }
    else if(std::string(argv[1]) == "-b"){
        // 如果第一个参数是 "-b"，实现高精度乘法，否则执行普通乘法
        std::string num1=argv[2];
        std::string num2=argv[3];
        std::cout << argv[2] << " * " << argv[3] << " = " << bigbigmul(num1,num2) << std::endl;
        return 0;
    }

    try {
        // 尝试将命令行参数转换为数字
        // std::stod (string to double) 函数可以自动识别并转换
        // 整数 (如 "2")、浮点数 (如 "3.1416") 和科学记数法 (如 "2.0e-2")。
        double num1 = std::stod(argv[1]);
        double num2 = std::stod(argv[2]);

        // 执行乘法计算
        double result = num1 * num2;

        // 按照要求格式打印表达式和结果
        std::cout << argv[1] << " * " << argv[2] << " = " << result << std::endl;

    } catch (const std::invalid_argument&) {
        // 如果 stod 的参数不是一个有效的数字字符串，会抛出 std::invalid_argument 异常。
        std::cerr << "输入不能被解析为一个数字！" << std::endl;
        return 1;
    } catch (const std::out_of_range&) {
        // 如果数字字符串代表的值超出了 double 的表示范围，会抛出 std::out_of_range 异常。
        std::cerr << "输入的数字超出范围！" << std::endl;
        return 1;
    }

    return 0;
}

std::string bigbigmul (const std::string &num1, const std::string &num2) {
    if(num1 == "0" || num2 == "0") return "0";
    // 处理乘法的逻辑
    int len1 = num1.size();
    int len2 = num2.size();
    std::vector<int> result(len1 + len2, 0);
    // 进行乘法计算
    for(int i=len1-1 ; i>=0 ; i--){
        for(int j=len2-1 ; j>=0 ; j--){
            int mul = (num1[i]-'0') * (num2[j]-'0');
            result[i+j+1] += mul;
        }
    }
    // 处理进位
    for(int i=len1+len2-1 ; i>0 ; i--){
        if(result[i] >= 10){
            result[i-1] += result[i] / 10;
            result[i] %= 10;
        }
    }
    // 构建结果字符串
    std::string res;
    int start = (result[0] == 0) ? 1 : 0; // 跳过前导零
    for(int i=start ; i<len1+len2 ; i++){
        res += (result[i] + '0');
    }
    return res; // 返回结果字符串
}
