#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

using namespace std;

const unsigned char ETX = 3;
const unsigned char delim = '|';

class FeedData
{
private:
    typedef struct
    {
        int     token;
        string  value;
    } TOKEN_VALUE_PAIR;

    istream*                            DataStream;
    vector<vector<TOKEN_VALUE_PAIR>>    Messages;
    size_t                              msg_count;

    typedef struct
    {
        vector<int> signature;
        size_t      count;
    } SIGNATURE_STAT;

    typedef struct
    {
        TOKEN_VALUE_PAIR            token3_value_pair;
        vector<SIGNATURE_STAT>      signatures;
    } TOKEN3_STAT;

    vector<TOKEN3_STAT>             statistics;

public:
    FeedData(string File)
    {
        ifstream ifs(File);
        DataStream = &ifs;
        msg_count = 0;
        get_messages(*DataStream);
        print_stats();
    }

private:
    bool get_token_pair(stringstream& message, int& token, string& value)
    {
        char    str[_MAX_PATH];
        message.getline(str, sizeof(str), delim);
        stringstream token_pair(str);
        char    eq;
        token_pair >> token;
        token_pair >> eq;
        token_pair >> value;
        return (message.rdstate() == ios::goodbit) ? true : false;
    }

    bool get_message(istream& s, stringstream& msg)
    {
        char line[_MAX_PATH];
        s >> ws;    // Trim leading whitespace
        s.getline(line, sizeof(line), ETX);
        msg = stringstream(line);
        return (s.rdstate() == ios::goodbit) ? true : false;
    }

    void get_messages(istream& s)
    {
        stringstream message;

        while(get_message(s, message))
        {
            vector<TOKEN_VALUE_PAIR> tv_pairs;
            stringstream token_pair;
            size_t token_count = 0;
            bool done = false;

            do
            {
                TOKEN_VALUE_PAIR tvp;
                get_token_pair(message, tvp.token, tvp.value);
                tv_pairs.push_back(tvp);
                ++token_count;

                if (message.rdstate() == ios::eofbit)
                {
                    done = true;
                }
            } while (!done);

            Messages.push_back(tv_pairs);
            ++msg_count;
        }
    }

    void print_stats()
    {
        for (size_t i = 0; i < Messages.size(); ++i)
        {
            TOKEN3_STAT     new_t3s;
            new_t3s.token3_value_pair.token = Messages[i][0].token;
            new_t3s.token3_value_pair.value = Messages[i][0].value;

            SIGNATURE_STAT  new_sig_stat;
            new_sig_stat.count = 1;

            for (size_t j = 0; j < Messages[i].size(); ++j)
            {
                new_sig_stat.signature.push_back(Messages[i][j].token);
            }

            new_t3s.signatures.push_back(new_sig_stat);

            bool found_t3s = false;

            for (size_t j = 0; !found_t3s && j < statistics.size(); ++j)
            {
                TOKEN3_STAT* t3s = &statistics[j];
                if (t3s->token3_value_pair.token == new_t3s.token3_value_pair.token &&
                    t3s->token3_value_pair.value == new_t3s.token3_value_pair.value)
                {
                    found_t3s = true;

                    // Insert new_sig_stat into t3s
                    bool found_sig_array = false;

                    for (size_t k = 0; !found_sig_array && k < t3s->signatures.size() ; ++k)
                    {
                        SIGNATURE_STAT *sig = &t3s->signatures[k];

                        if (sig->signature == new_sig_stat.signature)
                        {
                            found_sig_array = true;
                            ++sig->count;
                        }
                    }

                    if (!found_sig_array)
                    {
                        t3s->signatures.push_back(new_sig_stat);
                    }
                }
            }

            if (!found_t3s)
            {
                statistics.push_back(new_t3s);
            }
        }

        // Sort signatures by descending count
        for (size_t i = 0; i < statistics.size(); ++i)
        {
            sort(statistics[i].signatures.begin(), statistics[i].signatures.end(),
                [](const SIGNATURE_STAT& elem1, const SIGNATURE_STAT& elem2)
                {
                    if (elem1.count > elem2.count)
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                });
        }

        // Results to stdout
        for (size_t i = 0; i < statistics.size(); ++i)
        {
            cout << endl << statistics[i].token3_value_pair.token << '=' << statistics[i].token3_value_pair.value << ": unique signatures - " << statistics[i].signatures.size() << endl << endl;
            cout << "Top 3:" << endl;

            size_t j = 0;

            for (j = 0; j < 3; ++j)
            {
                if (j < statistics[i].signatures.size())
                {
                    cout << '[';
                    size_t k;

                    for (k = 0; k < statistics[i].signatures[j].signature.size(); ++k)
                    {
                        cout << statistics[i].signatures[j].signature[k];

                        if (k == 0 || k < statistics[i].signatures[j].signature.size() - 1)
                        {
                            cout << ',';
                        }
                    }
                    
                    cout << ']' << statistics[i].signatures[j].count << endl;
                }
                else
                {
                    cout << "N/A 0" << endl;
                }
            }
        }
    }
};

int main(int argc, char* argv[])
{
    FeedData fd(argv[1]);    

    return 0;
}
