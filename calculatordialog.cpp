#include "calculatordialog.h"
#include "ui_calculatordialog.h"

#include <QDebug>

CalculatorDialog::CalculatorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalculatorDialog)
{
    ui->setupUi(this);

    m_bFinish =false;

    setWindowTitle("Calculator");
    ui->resEdt->setReadOnly(true);
    ui->acceptBt->setStyleSheet("background-color:skyblue");
    ui->pushButton_23->setStyleSheet("background-color:orange");

    connect(this, &CalculatorDialog::buttonClicked, this, &CalculatorDialog::receiveClickedContent);
}

CalculatorDialog::~CalculatorDialog()
{
    delete ui;
}

QString CalculatorDialog::getExpression()
{
    return ui->lineEdit->text();
}

/**
 * @brief CalculatorDialog::doCalculating  计算表达式结果
 * @param expr
 * @param errStr
 * @return
 */
double CalculatorDialog::doCalculating(const QString &expr, QString *errStr)
{
    if(expr.isEmpty())  return 0.0;
    QQueue<QString> queue =splitExpression(expr);
    queue =convert2Postfix(queue);
    QString res =calculate(queue);

    bool ok;
    res.toDouble(&ok);
    if(ok){
        if(!m_errOutput.isEmpty()){
            *errStr =m_errOutput;
            m_errOutput.clear();
        }
        return res.toDouble();
    }
    else{
        QString str ="\ndoCalculating(): calculator returns an invaild result:" +res;
        *errStr +=str;
        qDebug()<<__PRETTY_FUNCTION__<<" :calculator returns an invaild result:"<<res;
        return 0.0;
    }
}

/**
 * @brief CalculatorDialog::judgeExpression 判断表达式是否合法
 * @param expr
 * @return
 */
bool CalculatorDialog::judgeExpression(const QString &expr)
{
    int bracket_cnt =0;
    for(int i =0; i < expr.length(); ++i)
    {
        QString leftStr =expr.left(i);
        const QChar ch =expr.at(i);
        if(ch >="0" && ch <="9")        //数字
        {
            QString temp =leftStr.right(1);
            if(temp ==")" ||temp =="$"){
                m_errStr ="Expression: wrong char before number.";
                return false;
            }
        }
        else if(ch ==".")       //小数点
        {
            QString temp =leftStr.right(1);
            if(temp.length()){
                if(temp <"0" || temp >"9"){
                     m_errStr ="Expression: wrong char before \'.\'.";
                    return false;
                }
                else{
                    for(int j =i -2; j >=0; --j){
                        if(expr.at(j) >="0" &&expr.at(j) <="9")  continue;
                        if(expr.at(j) =="."){
                             m_errStr ="Expression: use \'.\' incorrectly.";
                            return false;
                        }
                        else
                            break;
                    }
                }
            }
            else{
                 m_errStr ="Expression: wrong char before \'.\'.";
                return false;
            }
        }
        else if(ch =="+" ||ch =="*" ||ch =="/")         //加、乘、除号
        {
            QString temp =leftStr.right(1);
            if(temp.length()){
                if(isExpressionTail(temp.at(0)) ==false){
                     m_errStr ="Expression: wrong char before \'" +QString(ch) +"\'.";
                    return false;
                }
            }
            else{
                 m_errStr ="Expression: wrong char before \'" +QString(ch)+"\'.";
                return false;
            }
        }
        else if(ch =="-")           //减号，可作为负号
        {
            QString temp =leftStr.right(1);
            if(temp =="."){
                 m_errStr ="Expression: wrong char before \'-\'.";
                return false;
            }
            temp =leftStr.right(2);
            if(temp.length() ==2){
                if(matchCharacter(temp.at(0), "+-*/(") !=-1){
                    if(matchCharacter(temp.at(1), "+-*/") !=-1){
                         m_errStr ="Expression: wrong char before \'-\'.";
                        return false;
                    }
                }
            }
        }
        else if(ch =="(")           //左括号
        {
            QString temp =leftStr.right(1);
            if(temp.length()){
                if(isExpressionTail(temp.at(0)) ||temp =="."){
                     m_errStr ="Expression: wrong char before \'(\'.";
                    return false;
                }
            }
            bracket_cnt++;
        }
        else if(ch ==")")           //右括号
        {
            if(bracket_cnt ==0){
                 m_errStr ="Expression: use \')\' incorrectly.";
                return false;
            }
            QString temp =leftStr.right(1);
            if(temp.length()){
                if(!isExpressionTail(temp.at(0))){
                     m_errStr ="Expression: wrong char before \')\'.";
                    return false;
                }
            }
            else{
                 m_errStr ="Expression: wrong char before \')\'.";
                return false;
            }
            bracket_cnt--;
        }
        else if(ch =="&" ||ch =="|")            //逻辑与、逻辑或
        {
            if(i <expr.length() -1){
                QString temp =expr.at(i +1);
                if(temp !=ch){
                     m_errStr ="Expression: unknow char \'" +QString(ch)+"\'.";
                    return false;
                }
                temp =leftStr.right(1);
                if(temp.length()){
                    if(!isExpressionTail(temp.at(0))){
                         m_errStr ="Expression: wrong char before \'" +QString(ch)+"\'.";
                        return false;
                    }
                }
                else{
                     m_errStr ="Expression: wrong char before \'" +QString(ch)+"\'.";
                    return false;
                }
            }
            else{
                 m_errStr ="Expression: unknow char \'" +QString(ch)+"\'.";
                return false;
            }
            i++;
            continue;
        }
        else if(ch =="=" ||ch =="!")            //判断：相等、不等
        {
            if(i <expr.length() -1){
                QString temp =expr.at(i +1);
                if(temp !="="){
                     m_errStr ="Expression: unknow char \'" +QString(ch)+"\'.";
                    return false;
                }
                temp =leftStr.right(1);
                if(temp.length()){
                    if(!isExpressionTail(temp.at(0))){
                         m_errStr ="Expression: wrong char before \'" +QString(ch)+"\'.";
                        return false;
                    }
                }
                else{
                     m_errStr ="Expression: wrong char before \'" +QString(ch)+"\'.";
                    return false;
                }
            }
            else{
                 m_errStr ="Expression: unknow char \'" +QString(ch)+"\'.";
                return false;
            }
            i++;
            continue;
        }
        else if(ch =="<" ||ch ==">")            //判断：小于、大于、不大于、不小于
        {
            QString temp =leftStr.right(1);
            if(temp.length()){
                if(!isExpressionTail(temp.at(0))){
                     m_errStr ="Expression: wrong char before \'" +QString(ch)+"\'.";
                    return false;
                }
            }
            else{
                 m_errStr ="Expression: wrong char before \'" +QString(ch)+"\'.";
                return false;
            }
            if(i < expr.length() -1){
                if(expr.at(i +1) =="="){
                    i++;
                    continue;
                }
            }
        }
        else{
             m_errStr ="Expression: unknown char \'" +QString(ch)+"\'.";
            return false;
        }
    }

    if(expr.length()){
        int tail =expr.length() -1;
        if(!isExpressionTail(expr.at(tail))){
             m_errStr ="Expression: it's an incomplete expression.";
            return false;
        }
    }

    if(bracket_cnt !=0){
        m_errStr ="Expression: missing char \')\'.";
        return false;
    }

    return true;
}


/**
 * @brief CalculatorDialog::matchCharacter  判断字符串source中是否含target中的某个字符
 * @param source
 * @param target
 * @return
 */
int CalculatorDialog::matchCharacter(const QString &source, const char*target)
{
    for(int i=0;i<source.length();i++)
    {
        for(unsigned int j=0;j<strlen(target);j++)
            if(source[i]==target[j])
                return i;
    }
    return -1;
}

/**
 * @brief CalculatorDialog::isExpressionTail  判断是否位表达式结尾
 * @param ch
 * @return
 */
bool CalculatorDialog::isExpressionTail(const QChar &ch)
{
    if(ch >="0" &&ch <="9")  return true;
    if(ch ==")")  return true;
    return false;
}


/**
 * @brief CalculatorDialog::splitExpression  分离表达式的项和运算符
 * @param expr
 * @return
 */
QQueue<QString> CalculatorDialog::splitExpression(const QString &expr)
{
    QQueue<QString> queue;
    QString dataStr ="";

    for(int i =0; i < expr.length(); ++i)
    {
        const QChar ch =expr.at(i);
        if((ch >="0" &&ch <="9") ||ch ==".")
        {
            dataStr +=ch;
        }
        else if(ch =="-")
        {
            if(i ==0){
                dataStr +=ch;
            }
            else if(matchCharacter(expr.at(i -1), "(+-*/&|=<>") !=-1)
            {
                dataStr +=ch;
            }
            else{
                if(!dataStr.isEmpty()){
                    queue.enqueue(dataStr);
                    dataStr.clear();
                }
                queue.enqueue(ch);
            }
        }
        else if(matchCharacter(ch, "()+*/") !=-1)
        {
            if(!dataStr.isEmpty()){
                queue.enqueue(dataStr);
                dataStr.clear();
            }
            queue.enqueue(ch);
        }
        else if(ch =="&" ||ch =="|")
        {
            if(!dataStr.isEmpty()){
                queue.enqueue(dataStr);
                dataStr.clear();
            }
            if(i < expr.length() -1 &&expr.at(i +1) ==ch){
                QString opt =QString(ch) +QString(ch);
                queue.enqueue(opt);
                i++;
                continue;
            }
            else{
                qDebug()<<__PRETTY_FUNCTION__<<":spliting an expression with unknown char"<<ch<<".";
                queue.clear();
                return queue;
            }
        }
        else if(ch =="=" ||ch =="!")
        {
            if(!dataStr.isEmpty()){
                queue.enqueue(dataStr);
                dataStr.clear();
            }
            if(i < expr.length() -1 &&expr.at(i +1) =="="){
                QString opt =QString(ch) +"=";
                queue.enqueue(opt);
                i++;
                continue;
            }
            else{
                qDebug()<<__PRETTY_FUNCTION__<<":spliting an expression with unknown char"<<ch<<".";
                queue.clear();
                return queue;
            }
        }
        else if(ch =="<" ||ch ==">")
        {
            if(!dataStr.isEmpty())
            {
                queue.enqueue(dataStr);
                dataStr.clear();
            }
            if(i < expr.length() -1){
                if(expr.at(i +1) =="="){
                    QString opt =QString(ch) +"=";
                    queue.enqueue(opt);
                    i++;
                    continue;
                }
                else{
                    queue.enqueue(ch);
                }
            }
            else{
                queue.enqueue(ch);
            }
        }
    }

    if(!dataStr.isEmpty()){
        queue.enqueue(dataStr);
        dataStr.clear();
    }

    return queue;
}


/**
 * @brief CalculatorDialog::convert2Postfix  中缀转后缀
 * @param source
 * @return
 */
QQueue<QString> CalculatorDialog::convert2Postfix(QQueue<QString> &source)
{
    QQueue<QString> queue;
    QStack<QString> stack;
    QString dataStr;
    bool isNum;

    while(!source.isEmpty())
    {
        dataStr =source.dequeue();
        dataStr.toDouble(&isNum);
        if(isNum){
            queue.enqueue(dataStr);
        }
        else if(dataStr =="(")
        {
            stack.push(dataStr);
        }
        else if(dataStr ==")")
        {
            while(!stack.isEmpty() && stack.top() !="("){
                queue.enqueue(stack.pop());
            }
            if(stack.top() =="("){
                stack.pop();
            }
        }
        else{
            while(!stack.isEmpty() && stack.top() !="(" && priority(dataStr) <=priority(stack.top())){
                queue.enqueue(stack.pop());
            }
            stack.push(dataStr);
        }
    }

    while(!stack.isEmpty() && stack.top() !="("){
        queue.enqueue(stack.pop());
    }

    return queue;
}


/**
 * @brief CalculatorDialog::priority 定义运算符优先级
 * @param opt
 * @return
 */
int CalculatorDialog::priority(const QString &opt)
{
    int base =0;
    if(opt =="&&" ||opt =="||")  return base +1;
    if(opt =="==" ||opt =="!=" ||opt =="<" ||opt ==">"
            ||opt =="<=" ||opt ==">=")
        return base +2;
    if(opt =="+" ||opt =="-")  return base +3;
    if(opt =="*" ||opt =="/")  return base +4;
    if(opt =="(" ||opt ==")")  return base +5;
    return base;
}


/**
 * @brief CalculatorDialog::optCalculate  运算符操作
 * @param opt
 * @param l
 * @param r
 * @return
 */
QString CalculatorDialog::optCalculate(const QString &opt, const QString &l, const QString &r)
{
    double res =0.0;
    double left =l.toDouble();
    double right =r.toDouble();

    if(opt =="+")
    {
        res =left +right;
    }
    else if(opt =="-")
    {
        res =left -right;
    }
    else if(opt =="*")
    {
        res =left *right;
    }
    else if(opt =="/")
    {
        assert(right !=0.0);
        res =left /right;
    }
    else if(opt =="==")
    {
        res =left ==right;
    }
    else if(opt =="!=")
    {
        res =left !=right;
    }
    else if(opt =="<")
    {
        res =left <right;
    }
    else if(opt ==">")
    {
        res =left >right;
    }
    else if(opt =="<=")
    {
        res =left <=right;
    }
    else if(opt ==">=")
    {
        res =left >=right;
    }
    else if(opt =="&&")
    {
        res =left &&right;
    }
    else if(opt =="||")
    {
        res =left ||right;
    }

    return QString::number(res);
}


/**
 * @brief CalculatorDialog::calculate  计算表达式
 * @param postfix_queue
 * @return
 */
QString CalculatorDialog::calculate(QQueue<QString> &postfix_queue)
{
    QStack<QString> stack;
    QString dataStr;
    bool isNum;
    QString res;

    if(postfix_queue.isEmpty())  return QString::number(0.0);

    while(!postfix_queue.isEmpty())
    {
        dataStr =postfix_queue.dequeue();
        dataStr.toDouble(&isNum);

        if(isNum)
        {
            stack.push(dataStr);
        }
        else
        {
            if(stack.size() <2)
            {
                qDebug()<<__PRETTY_FUNCTION__<<":incorrect queue of the postfix expression.";
                return "Error";
            }
            QString right =stack.pop();
            QString left =stack.pop();
            res =optCalculate(dataStr, left, right);
            stack.push(res);
        }
    }
    if(stack.size() ==1)
    {
        return stack.pop();
    }
    else
    {
        qDebug()<<__PRETTY_FUNCTION__<<":incorrect queue of the postfix expression.";
        return "Error";
    }
}

void CalculatorDialog::on_acceptBt_clicked()
{
    QString str =ui->lineEdit->text();
    QString res;
    if(judgeExpression(str))
    {
        QQueue<QString> queue =splitExpression(str);
        queue =convert2Postfix(queue);
        res =calculate(queue);
        m_bFinish =true;
    }
    else{
        res =m_errStr;
    }
    ui->resEdt->setText(res);
}

void CalculatorDialog::on_selectData(const QString &text)
{
    QString str =ui->lineEdit->text() +text;
    ui->lineEdit->setText(str);
    ui->lineEdit->setFocus();
}

void CalculatorDialog::receiveClickedContent(const QString &text)
{
    if(m_bFinish){
        ui->lineEdit->clear();
        m_bFinish =false;
    }
    ui->lineEdit->insert(text);
}

void CalculatorDialog::on_pushButton_clicked()
{
    emit buttonClicked("00");
}

void CalculatorDialog::on_pushButton_0_clicked()
{
    emit buttonClicked("0");
}

void CalculatorDialog::on_pushButton_1_clicked()
{
    emit buttonClicked("1");
}

void CalculatorDialog::on_pushButton_2_clicked()
{
    emit buttonClicked("2");
}

void CalculatorDialog::on_pushButton_3_clicked()
{
    emit buttonClicked("3");
}

void CalculatorDialog::on_pushButton_4_clicked()
{
    emit buttonClicked("4");
}


void CalculatorDialog::on_pushButton_5_clicked()
{
    emit buttonClicked("5");
}

void CalculatorDialog::on_pushButton_6_clicked()
{
   emit buttonClicked("6");
}

void CalculatorDialog::on_pushButton_7_clicked()
{
    emit buttonClicked("7");
}

void CalculatorDialog::on_pushButton_8_clicked()
{
    emit buttonClicked("8");
}


void CalculatorDialog::on_pushButton_9_clicked()
{
    emit buttonClicked("9");
}

void CalculatorDialog::on_pushButton_10_clicked()
{
    emit buttonClicked(".");
}

void CalculatorDialog::on_pushButton_11_clicked()
{
    emit buttonClicked("*");
}

void CalculatorDialog::on_pushButton_12_clicked()
{
    emit buttonClicked("/");
}

void CalculatorDialog::on_pushButton_13_clicked()
{
    emit buttonClicked("+");
}

void CalculatorDialog::on_pushButton_14_clicked()
{
    emit buttonClicked("-");
}

void CalculatorDialog::on_pushButton_15_clicked()
{
    emit buttonClicked("&&");
}

void CalculatorDialog::on_pushButton_16_clicked()
{
    emit buttonClicked("||");
}

void CalculatorDialog::on_pushButton_17_clicked()
{
    emit buttonClicked("==");
}

void CalculatorDialog::on_pushButton_18_clicked()
{
    emit buttonClicked("!=");
}

void CalculatorDialog::on_pushButton_19_clicked()
{
    emit buttonClicked("<");
}

void CalculatorDialog::on_pushButton_20_clicked()
{
    emit buttonClicked(">");
}

void CalculatorDialog::on_pushButton_21_clicked()
{
    emit buttonClicked("<=");
}

void CalculatorDialog::on_pushButton_22_clicked()
{
    emit buttonClicked(">=");
}

void CalculatorDialog::on_pushButton_23_clicked()
{
    ui->lineEdit->clear();
    ui->resEdt->clear();
}

void CalculatorDialog::on_pushButton_26_clicked()
{
    emit buttonClicked("(");
}

void CalculatorDialog::on_pushButton_27_clicked()
{
    emit buttonClicked(")");
}

void CalculatorDialog::on_pushButton_25_clicked()
{
    if(ui->lineEdit->hasSelectedText())
        ui->lineEdit->insert("");
    else
        ui->lineEdit->backspace();
}
