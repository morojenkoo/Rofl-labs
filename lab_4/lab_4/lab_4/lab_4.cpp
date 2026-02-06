#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <functional>
#include <random>
#include <unordered_map>

using namespace std::chrono;
using namespace std;

// S -> T a a S b b T   { T1.a < T2.a }
// T -> a b S           { T.a := 0 }
// T -> a T             { T0.a := T1.a + 1 }
// T -> a               { T.a := 0 }
// S -> a b a
// S -> a a a


vector<pair<int, int>> naive_T(int pos, const string& s, vector<pair<int, int>>& attributes);
vector<int> naive_S(int pos, const string& s, vector<int>& result);


//Парсим S
vector<int> naive_S(int pos, const string& s, vector<int>& result)
{
    //result - вектор позиций, где мы завершили разбор S
    int attr1 = 0, attr2 = 0;
    //t1_results - вектор пар (позиция, атрибут), где мы возможно завершили работу T и посчитали атрибут
    vector<pair<int, int>> t1_results;
    t1_results = naive_T(pos, s, t1_results);

    //Перебираем все возможные завершения разбора нетерминала T
    for (const auto& t1 : t1_results)
    {
        int pos1 = t1.first;
        attr1 = t1.second;
        // S -> T  `aa`  SbbT
        if (pos1 + 1 >= s.size() || s[pos1] != 'a' || s[pos1 + 1] != 'a')
            continue;
        
        int pos2 = pos1 + 2;

        // S -> Taa  `S`  bbT'
        vector<int> s_results;
        s_results = naive_S(pos2, s, s_results);

        // S -> TaaS  `bb`  T
        for (int pos3 : s_results) {
            if (pos3 + 1 >= s.size() || s[pos3] != 'b' || s[pos3 + 1] != 'b')
                continue;
            
            int pos4 = pos3 + 2;
            
            // S->TaaSbb  `T`
            vector<pair<int, int>> t2_results;
            t2_results = naive_T(pos4, s, t2_results);
            for (const auto& t2 : t2_results)
            {
                int pos5 = t2.first;
                attr2 = t2.second;
                if (attr1 < attr2)
                    result.push_back(pos5);
            }
        }
    }
    //S -> aba
    if (pos + 2 < s.size() && s[pos] == 'a' && s[pos + 1] == 'b' && s[pos + 2] == 'a')
    {
        result.push_back(pos + 3);
    }
    //S-> aaa
    if (pos + 2 < s.size() && s[pos] == 'a' && s[pos + 1] == 'a' && s[pos + 2] == 'a')
    {
        result.push_back(pos + 3);
    }

    return result;
}

//Парсим T. Возвращаем вектор пар (позиция, в которой завершились, атрибут)
vector<pair<int, int>> naive_T(int pos, const string& s, vector<pair<int, int>>& attributes)
{

    int n = s.size();
    // T -> a b S
    if (pos+1 < n && s[pos] == 'a' && s[pos+1] == 'b')
    {
        int pos2 = pos + 2;

        //Вызвали парсер S. Там атрибуты считаются по внутреннему правилу и не влияют на внешний разбор
        vector<int> s_results;
        s_results = naive_S(pos2, s, s_results);

        //Получили все возможные варианты конца разбора S. Зануляем атрибут у этих позиций.
        for (int end_pos : s_results)
            attributes.push_back({end_pos, 0});
    }
    if (pos < n && s[pos] == 'a')
    {
        //T -> aT, возвращает все возможные варианты, где заканчивается этот блок
        vector<pair<int, int>> t_results = naive_T(pos + 1, s, attributes);
        for (const auto& t : t_results)
            attributes.push_back({t.first, t.second+1});
    }
    if (pos < n && s[pos] == 'a')
        //T -> a занулили атрибут
        attributes.push_back({pos+1, 0});
    return attributes;
}

// Хэш-функция для пары (int, string)
struct PairHash
{
    size_t operator()(const pair<int, string>& p) const
    {
        auto h1 = hash<int>{}(p.first);
        auto h2 = hash<string>{}(p.second);
        return h1 ^ (h2 << 1); 
    }
};

// Мемоизация для T: ключ - пара (позиция, строка), значение - вектор пар (позиция, атрибут)
unordered_map<pair<int, string>, vector<pair<int, int>>, PairHash> memo_T;

// Мемоизация для S: ключ - пара (позиция, строка), значение - вектор позиций
unordered_map<pair<int, string>, vector<int>, PairHash> memo_S;

vector<pair<int, int>> optimized_T(int pos, const string& s);
vector<int> optimized_S(int pos, const string& s);

// Оптимизированный парсер 
vector<int> optimized_S(int pos, const string& s) 
{
    auto key = make_pair(pos, s);
    if (memo_S.count(key))
        return memo_S[key];

    vector<int> result;
    int attr1 = 0, attr2 = 0;
    vector<pair<int, int>> t1_results;
    t1_results = optimized_T(pos, s);

    for (const auto& t1 : t1_results) 
    {
        int pos1 = t1.first;
        attr1 = t1.second;
        if (pos1 + 1 >= s.size() || s[pos1] != 'a' || s[pos1 + 1] != 'a') 
            continue;

        int pos2 = pos1 + 2;
        vector<int> s_results;
        s_results = optimized_S(pos2, s);

        for (int pos3 : s_results) 
        {
            if (pos3 + 1 >= s.size() || s[pos3] != 'b' || s[pos3 + 1] != 'b')
                continue;
            
            int pos4 = pos3 + 2;
            vector<pair<int, int>> t2_results;
            t2_results = optimized_T(pos4, s);
            for (const auto& t2 : t2_results) 
            {
                int pos5 = t2.first;
                attr2 = t2.second;
                if (attr1 < attr2)
                    result.push_back(pos5);
            }
        }
    }
    if (pos + 2 < s.size() && s[pos] == 'a' && s[pos + 1] == 'b' && s[pos + 2] == 'a') 
    {
        result.push_back(pos + 3);
    }

    if (pos + 2 < s.size() && s[pos] == 'a' && s[pos + 1] == 'a' && s[pos + 2] == 'a') 
    {
        result.push_back(pos + 3);
    }
    memo_S[key] = result;
    return result;
}

vector<pair<int, int>> optimized_T(int pos, const string& s) 
{
    auto key = make_pair(pos, s);
    if (memo_T.count(key)) 
        return memo_T[key];

    vector<pair<int, int>> attributes;
    int n = s.size();

    if (pos+1 < n && s[pos] == 'a' && s[pos+1] == 'b') {
        int pos2 = pos + 2;
        vector<int> s_results;
        s_results = optimized_S(pos2, s);

        for (int end_pos : s_results)
            attributes.push_back({end_pos, 0});
    }

    if (pos < n && s[pos] == 'a') {
        vector<pair<int, int>> t_results = optimized_T(pos + 1, s);
        for (const auto& t : t_results)
            attributes.push_back({t.first, t.second+1});
    }

    if (pos < n && s[pos] == 'a')
        attributes.push_back({pos+1, 0});

    memo_T[key] = attributes;
    return attributes;
}

bool check(const vector<int>& v, int len)
{
    for (int p : v)
        if (p == len)
            return true;
    return false;
}

string generate_T1(int attr_value);
string generate_T2(int attr_value);

string generate_S()
{
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> rule_dis(0, 2);
    int rule = rule_dis(gen);
    switch (rule)
    {
        case 0: return "aba"; // S -> aba
        case 1: return "aaa"; // S -> aaa
        case 2:
        { // S -> TaaSbbT
            int t1_attr = 0;
            int t2_attr = t1_attr + 1 + (gen() % 4);

            string t1_str = generate_T1(t1_attr);
            string inner_s_str = generate_S();
            string t2_str = generate_T2(t2_attr);

            return t1_str + "aa" + inner_s_str + "bb" + t2_str;
        }
    }
    return "";
}

string generate_T1(int attr_value)
{
    static mt19937 gen(random_device{}());
    
    if (attr_value == 0)
    {
        uniform_int_distribution<> dis(0, 1);
        int choice = dis(gen);
        if (choice == 0)
            return "ab" + generate_S(); // T -> abS
        else
            return "a"; // T -> a
    }
    else
        return "a" + generate_T1(attr_value - 1);
}
string generate_T2(int attr_value)
{
    static mt19937 gen(random_device{}());
    if (attr_value == 0)
        return "a";
    return "a" + generate_T2(attr_value - 1);
}

// Так красивее :)
string generate_random_string_from_language()
{
    return generate_S();
}

//Рандомная строка рандомной длины. Допустим, она не принадлежит языку...
string generate_random_string()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 1000);
    int length = dist(gen);
    string result;
    uniform_int_distribution<> dis(0, 1);
    for (int i = 0; i < length; i++)
        result += dis(gen) == 0 ? 'a' : 'b';
    return result;
}

//Если проверяем слово, заведомо принадлежащее языку - смотрим, правильно ли его распознают оба парсера
//Если рандомное - смотрим, совпадают ли парсеры в своём мнении
//На большом количестве тестов может быть сгенерируются несколько рандомных строк из языка
vector<pair<int, double>> naive_in_language_times;
vector<pair<int, double>> optimized_in_language_times;
vector<pair<int, double>> naive_random_times;
vector<pair<int, double>> optimized_random_times;

bool fuzz(string& s, bool in_language)
{
    memo_S.clear();
    memo_T.clear();
    vector<int> result;
    int len = s.size();
    auto start_naive = high_resolution_clock::now();
    bool naive_res = check(naive_S(0, s, result), len);
    auto end_naive = high_resolution_clock::now();
    double naive_time = duration<double, milli>(end_naive - start_naive).count() / 1000.0;

    auto start_optimized = high_resolution_clock::now();
    bool optimized_res = check(optimized_S(0, s), len);
    auto end_optimized = high_resolution_clock::now();
    double optimized_time = duration<double, milli>(end_optimized - start_optimized).count() / 1000.0;
    if (in_language)
    {
        if (naive_res != in_language || optimized_res != in_language)
        {
            cout << s << endl;
            cout << "in_language naive_res = "<< naive_res << endl;
            cout << "in_language optimized_res = " << optimized_res << endl;
            return false;
        }
        naive_in_language_times.push_back({len, naive_time});
        optimized_in_language_times.push_back({len, optimized_time});
    }
    else
    {
        if (naive_res != optimized_res)
        {
            cout << s << endl;
            cout << "not in_language naive_res = "<< naive_res << endl;
            cout << "not in_language optimized_res = " << optimized_res << endl;
            return false;
        }
        naive_random_times.push_back({len, naive_time});
        optimized_random_times.push_back({len, optimized_time});
    }
    return true;
}

int main()
{
    string yes, no;
    bool y = true, n = true;

    //Чтобы тестировать разные строки, будем запоминать уже протестированные в этом векторе
    vector <string> tests;

    //100 тестов, чтобы не ждать долго результата
    for (int i = 0; i < 100; i++)
    {
        yes = generate_random_string_from_language();
        bool in_language = 1;
        
        //На словах длины больше 100 наивный парсер крепенько так обдумывает
        if (yes.size() > 100 || find(tests.begin(), tests.end(), yes) != tests.end())
            continue;
        tests.push_back(yes);
        if (!fuzz(yes, in_language))
        {
            y = false;
            break;
        }
    }
    tests.clear();
    
    //Здесь можно сделать намного больше тестов и работать будет быстро, но опять так красивее :)
    for (int i = 0; i < 100; i++)
    {
        no = generate_random_string();
        bool in_language = 0;
        if (find(tests.begin(), tests.end(), yes) != tests.end())
            continue;
        if (!fuzz(no, in_language))
        {
            n = false;
            break;
        }
    }
    
    if (y)
        cout << "in language good" << endl;
    else
        cout << "in language bad" << endl;
    if (n)
        cout << "not in language good" << endl;
    else
        cout << "not in language bad" << endl;
    
    // Выводим данные о времени, нужно для построения графиков
    for (const auto& p : naive_in_language_times) 
        cout << "naive_in " << p.first << " " << p.second << endl;
    for (const auto& p : optimized_in_language_times) 
        cout << "opt_in " << p.first << " " << p.second << endl;
    for (const auto& p : naive_random_times) 
        cout << "naive_rand " << p.first << " " << p.second << endl;
    for (const auto& p : optimized_random_times) 
        cout << "opt_rand " << p.first << " " << p.second << endl;
    
    return 0;
}
