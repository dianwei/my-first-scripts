#include <iostream> 
#include <string>   // 用于 std::stod 和 std::string
#include <stdexcept> // 用于捕获 std::stod 可能抛出的异常 (std::invalid_argument, std::out_of_range)
#include <vector>


// 高精度浮点数乘法
typedef struct{
    std::string integerPart;  // 整数部分
    std::string fractionalPart; // 小数部分
    int exponent; // 科学计数法指数
    bool negative; // 是否为负数
}HighPrecisionFloat;

std::string bigbigmul (const std::string &num1, const std::string &num2);
HighPrecisionFloat parseString(const std::string& str);
void HighPrecisionMultiply(const HighPrecisionFloat& num1, const HighPrecisionFloat& num2);


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
        HighPrecisionFloat hpf1 = parseString(num1);
        HighPrecisionFloat hpf2 = parseString(num2);
        HighPrecisionMultiply(hpf1, hpf2);
        // std::cout << argv[2] << " * " << argv[3] << " = " << bigbigmul(num1,num2) << std::endl;
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
    // 按位进行乘法计算
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
    for(int i=0 ; i<len1+len2 ; i++){
        res += (result[i] + '0');
    }
    return res; // 返回结果字符串
}

HighPrecisionFloat parseString(const std::string& str){
    HighPrecisionFloat hpf;
    size_t pos = 0;

    // 处理符号
    if (!str.empty() && str[pos] == '-') {
        hpf.negative = true;
        pos++;
    } else if(!str.empty() && str[pos] == '+') {
        hpf.negative = false;
        pos++;
    }

    // 处理整数部分和小数部分
    size_t decimalPos = str.find('.', pos);
    size_t expPos = str.find_first_of("eE", pos);

    if (decimalPos != std::string::npos) {
        hpf.integerPart = str.substr(pos, decimalPos - pos);
        if (expPos != std::string::npos) {
            hpf.fractionalPart = str.substr(decimalPos + 1, expPos - decimalPos - 1);
            hpf.exponent = std::stoi(str.substr(expPos + 1));
        } else {
            hpf.fractionalPart = str.substr(decimalPos + 1);
            hpf.exponent = 0;
        }
    } else {
        if (expPos != std::string::npos) {
            hpf.integerPart = str.substr(pos, expPos - pos);
            hpf.fractionalPart = "";
            hpf.exponent = std::stoi(str.substr(expPos + 1));
        } else {
            hpf.integerPart = str.substr(pos);
            hpf.fractionalPart = "";
            hpf.exponent = 0;
        }
    }

    // 输出解析结果（调试用）
    std::cout << "Integer Part: " << hpf.integerPart << std::endl;
    std::cout << "Fractional Part: " << hpf.fractionalPart << std::endl;
    std::cout << "Exponent: " << hpf.exponent << std::endl;
    std::cout << "Negative: " << (hpf.negative ? "Yes" : "No") << std::endl;
    
    return hpf;
} 

void HighPrecisionMultiply(const HighPrecisionFloat& num1, const HighPrecisionFloat& num2) {
    // 实现高精度浮点数乘法的逻辑
    // 这部分代码需要处理整数部分、小数部分和指数的乘法
    // 以及结果的规范化和格式化输出
    std::string num1_full = num1.integerPart + num1.fractionalPart;
    std::string num2_full = num2.integerPart + num2.fractionalPart;
    std::string result = bigbigmul(num1_full, num2_full);
    size_t decimal_places = num1.integerPart.size() + num2.integerPart.size();
    std::cout << "Decimal places to insert: " << decimal_places << std::endl; // 调试输出
    std::cout << "Raw multiplication result: " << result << std::endl;// 调试输出
    // 插入小数点
    if(result.size() > decimal_places) {
        result.insert(decimal_places, ".");
    } 
    int result_exponent = num1.exponent + num2.exponent ;
    // 处理结果的符号
    bool result_negative = num1.negative ^ num2.negative;
    // 这里可以添加更多的代码来格式化和输出结果
    std::cout << "Result: " << (result_negative ? "-" : "") << result << "e" << result_exponent << std::endl;
}