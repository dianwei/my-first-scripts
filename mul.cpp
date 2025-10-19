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

std::string bigbigmul (const std::string &num1, const std::string &num2);// 大数乘法函数声明
HighPrecisionFloat parseString(const std::string& str);// 解析字符串为高精度浮点数
void HighPrecisionMultiply(const HighPrecisionFloat& num1, const HighPrecisionFloat& num2,bool useScientific);// 高精度乘法函数声明


int main(int argc, char* argv[]) {
    // 检查命令行参数数量
    bool useScientific = false;
    bool useHighPrecision = false;
    std::vector<std::string> numbers;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-s") {
            useScientific = true;
        } else if (arg == "-h") {
            useHighPrecision = true;
        } else if (arg == "--help") {
            std::cout << "用法: " << argv[0] << " [选项] <数字1> <数字2>" << std::endl;
            std::cout << "选项:" << std::endl;
            std::cout << "  -s               高精度计算下使用科学计数法输出" << std::endl;
            std::cout << "  -h               使用高精度计算" << std::endl;
            std::cout << "  --help           显示此帮助信息" << std::endl;
            return 0;
        } else {
            numbers.push_back(arg);
        }
    }
    // 确保有且仅有两个数字参数
    if (numbers.size() != 2) {
        std::cerr << "错误: 需要提供两个数字进行乘法运算。" << std::endl;
        std::cerr << "使用 --help 查看用法说明。" << std::endl;
        return 1;
    }
    // 如果启用高精度计算
    if (useHighPrecision) {
        HighPrecisionFloat num1 = parseString(numbers[0]);
        HighPrecisionFloat num2 = parseString(numbers[1]);
        HighPrecisionMultiply(num1, num2, useScientific);
        return 0;
    }

    try {
        // 尝试将命令行参数转换为数字
        // std::stod (string to double) 函数可以自动识别并转换
        // 整数 (如 "2")、浮点数 (如 "3.1416") 和科学记数法 (如 "2.0e-2")。
    double num1 = std::stod(numbers[0]);
    double num2 = std::stod(numbers[1]);

    // 执行乘法计算
    double result = num1 * num2;

    // 按照要求格式打印表达式和结果
    std::cout << numbers[0] << " * " << numbers[1] << " = " << result << std::endl;

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
    // std::cout << "Integer Part: " << hpf.integerPart << std::endl;
    // std::cout << "Fractional Part: " << hpf.fractionalPart << std::endl;
    // std::cout << "Exponent: " << hpf.exponent << std::endl;
    // std::cout << "Negative: " << (hpf.negative ? "Yes" : "No") << std::endl;
    
    return hpf;
} 

void HighPrecisionMultiply(const HighPrecisionFloat& num1, const HighPrecisionFloat& num2,bool useScientific) {
    // 实现高精度浮点数乘法的逻辑
    // 这部分代码需要处理整数部分、小数部分和指数的乘法
    // 以及结果的规范化和格式化输出
    std::string num1_full = num1.integerPart + num1.fractionalPart;
    std::string num2_full = num2.integerPart + num2.fractionalPart;
    std::string result = bigbigmul(num1_full, num2_full);
    int decimal_places = num1.integerPart.size() + num2.integerPart.size();// 小数点位置
    std::cout << "Raw multiplication result: " << result << std::endl;// 调试输出
    // 处理结果为零的情况
    if(result == "0"){
        std::cout << "Result: 0" << std::endl;
        return ;
    }
    // 跳过前导零
    if(result[0]=='0'){
        result=result.substr(1);
        decimal_places--;
    }

    int result_exponent = num1.exponent + num2.exponent ;
    // 处理结果的符号
    bool result_negative = num1.negative ^ num2.negative;
    // 这里可以添加更多的代码来格式化和输出结果
    //std::cout << "Result: " << (result_negative ? "-" : "") << result << "e" << result_exponent << std::endl;
    if(useScientific){
        // 这里添加科学计数法格式化输出的代码
        if(decimal_places > 1 ){
            result.insert(1,".");
            result_exponent += decimal_places - 1;
        }
        else if(result[0] == '0' && result.size() > decimal_places) {
            if(result.length() > 2)result.insert(decimal_places+1, ".");
            result_exponent -= 1;
            result.erase(0,1);
        }
        else if(result.size() > decimal_places) {
            result.insert(decimal_places, ".");
        }
        std::cout << "Result: " << (result_negative ? "-" : "") << result << "e" << result_exponent << std::endl;
    }
    else{
        // 普通格式输出
        if(decimal_places + result_exponent <= 0){
            // 小数点在最前面
            result.insert(0,"0.");
            result.insert(2,-(decimal_places + result_exponent),'0');
        }
        else if(result_exponent < 0){
            // 小数点在中间
            result.insert(result.size() + result_exponent - decimal_places,".");
        }
        else if(result_exponent > 0){
            // 小数点在后面
            result.append(result_exponent,'0');
        }
        else if(result_exponent == 0 && decimal_places < result.size()){
            result.insert(decimal_places,".");
        }
        // 跳过前导零
        while(result[0]=='0' && result.size()>1 && result[1]!='.'){
            result=result.substr(1);
        }
        std::cout << "Result: " << (result_negative ? "-" : "") << result << std::endl;
    }
}