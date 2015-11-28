#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <vector>

using namespace std;

class Rope {
public:
    virtual void insert(int value, size_t index) = 0;
    virtual void assign(int value, size_t index) = 0;
    virtual long long subsegmentSum(size_t left, size_t right) = 0;
    virtual bool nextPermutation(size_t left, size_t right) = 0;
};

class Treap: public Rope {
private:
    class TreapVertex {
        public:
            int key,  priority, value;
            int max_prefix, max_suffix;
            long long sum;
            bool reversed;
            TreapVertex *left, *right, *left_child, *right_child;
            int getKey() {
                if (this)
                    return key;
                else
                    return 0;
            }
            long long getSum() {
                if (this)
                    return sum;
                else
                    return 0;
            }
            void updateVertex() {
                if (this) {
                    push();
                    key = 1 + left->getKey() + right->getKey();
                    sum = value + left->getSum() + right->getSum();
                    if (left) {
                        left->updateVertex();
                        left_child = left->left_child;
                        max_prefix = left->max_prefix;
                        if (left->max_prefix == left->key && left->right_child->value <= value) {
                            max_prefix += 1;
                            if (right)
                                right->updateVertex();
                            if (right && right->left_child->value >= value)
                                max_prefix += right->max_prefix;
                        }
                    }
                    else
                        left_child = this, max_prefix = 1;
                    if (right) {
                        right->updateVertex();
                        right_child = right->right_child;
                        max_suffix = right->max_suffix;
                        if (right->max_suffix == right->key && right->left_child->value <= value) {
                            max_suffix += 1;
                            if (left)
                                left->updateVertex();
                            if (left && left->right_child->value <= value)
                                max_suffix += left->max_suffix;
                        }
                    }
                    else
                        right_child = this, max_suffix = 1;
                }
            }
            void push() {
                if (this && reversed) {
                    reversed = false;
                    swap(left, right);
                    swap(left_child, right_child);
                    swap(max_prefix, max_suffix);
                    if (left)
                        left->reversed ^= true;
                    if (right)
                        right->reversed ^= true;
                }
            }
            TreapVertex(int val) {
                left = right = NULL;
                max_prefix = max_suffix = 1;
                left_child = right_child = this;
                key = 1;
                value = sum = val;
                priority = rand();
                reversed = false;
            }
            ~TreapVertex() {
                if (left)
                    delete left;
                if (right)
                    delete right;
            }
    };
    TreapVertex *vertex;
    void merge(TreapVertex *&destination, TreapVertex *left_vertex, TreapVertex *right_vertex) {
        left_vertex->push();
        right_vertex->push();
        if (!left_vertex || !right_vertex)
            destination = left_vertex ? left_vertex : right_vertex;
        else
            if (left_vertex->priority > right_vertex->priority)
                merge(left_vertex->right, left_vertex->right, right_vertex), destination = left_vertex;
            else
                merge(right_vertex->left, left_vertex, right_vertex->left), destination = right_vertex;
        destination->updateVertex();
    }
    void split(TreapVertex *vertex, TreapVertex *&left_destination, TreapVertex *&right_destination, int key, int added = 0) {
        if (!vertex) {
            left_destination = right_destination = NULL;
            return;
        }
        vertex->push();
        int current_key = added + vertex->left->getKey();
        if (key <= current_key) {
            split(vertex->left, left_destination, vertex->left, key, added);
            right_destination = vertex;
        }
        else {
            split(vertex->right, vertex->right, right_destination, key, current_key + 1);
            left_destination = vertex;
        }
        vertex->updateVertex();
    }
    void splitByValue(TreapVertex *vertex, TreapVertex *&left_destination, TreapVertex *&right_destination, int value) {
        if (!this) {
            left_destination = right_destination = NULL;
            return;
        }
        vertex->push();
        if (vertex->value <= value) {
            splitByValue(vertex->right, vertex->right, right_destination, value);
            left_destination = vertex;
        }
        else {
            splitByValue(vertex->left, left_destination, vertex->left, value);
            right_destination = vertex;
        }
        vertex->updateVertex();
    }
    TreapVertex *replaceGreater(TreapVertex * vertex, TreapVertex * elem) {
        TreapVertex *left, *middle, *right;
        splitByValue(vertex, left, right, elem->value);
        split(right, middle, right, 1);
        merge(middle, middle, right);
        merge(left, left, middle);
        return middle;

    }
public:
    void insert(int value, size_t index) {
        TreapVertex *left, *right;
        TreapVertex *middle = new TreapVertex(value);
        split(vertex, left, right, index);
        merge(left, left, middle);
        merge(vertex, left, right);
    }

    void assign(int value, size_t index) {
        TreapVertex *left, *middle, *right;
        split(vertex, left, middle, index);
        split(middle, middle, right, 1);
        middle->priority = value;
        merge(left, left, middle);
        merge(vertex, left, right);
    }
    long long subsegmentSum(size_t left_index, size_t right_index) {
        TreapVertex *left, *middle, *right;
        split(vertex, middle, right, right_index);
        split(middle, left, middle, left_index);
        long long sum = middle->getSum();
        merge(left, left, middle);
        merge(vertex, left, right);
        return sum;
    }
    bool nextPermutation(size_t left_index, size_t right_index) {
        TreapVertex *left, *middle, *right, *temp_left, *temp_right;
        TreapVertex *left_pos, *right_pos;
        bool return_value = true;
        split(vertex, middle, right, right_index);
        split(middle, left, middle, left_index);
        if (middle->max_suffix == middle->key) {
            middle->reversed ^= 1;
            return_value = false;
        }
        else {
            split(middle, temp_left, temp_right, middle->key - middle->max_prefix);
            temp_right->reversed ^= 1;
            temp_right->push();
            if (temp_left) {
                split(temp_left, temp_left, left_pos, temp_left->key - 1);
                right_pos = replaceGreater(temp_right, left_pos);
                merge(temp_left, temp_left, left_pos);
            }

        }
        merge(left, left, middle);
        merge(vertex, left, right);
        return return_value;
    }
};

class NaiveRope: public Rope {
private:
    vector<int> v;
public:
    void insert(int value, size_t index) {
        v.push_back(0);
        for (int i = v.size() - 1; i > index; i--)
            v[i] = v[i - 1];
        v[index] = value;
    }
    void assign(int value, size_t index) {
        v[index] = value;
    }
    long long subsegmentSum(size_t left, size_t right) {
        long long sum = 0;
        for (int i = left; i < right; i++)
            sum += v[i];
        return sum;
    }
    bool nextPermutation(size_t left, size_t right) {
        return next_permutation(v.begin() + left, v.begin() + right);
    }
};

class Test {
public:
    class Command {
    public:
        enum CommandType {INSERT, ASSIGN, SUM, PERMUTATION} ;
        int left, right, value;
        CommandType type;
    };
    vector<Command> commands;
    vector<int> run(Rope * rope) {
        vector<int> answer;
        int rope_length = 0;
        for (int i = 0; i < commands.size(); i++)
            switch(commands[i].type) {
                case Command::INSERT:
                    rope->insert(commands[i].value, commands[i].left);
                    rope_length++;
                    break;
                case Command::ASSIGN:
                    rope->assign(commands[i].value, commands[i].left);
                    break;
                case Command::SUM:
                    answer.push_back(rope->subsegmentSum(commands[i].left, commands[i].right));
                    break;
                case Command::PERMUTATION:
                    rope->nextPermutation(commands[i].left, commands[i].right);
                    break;
            }
        for (int i = 0; i < rope_length; i++)
            answer.push_back(rope->subsegmentSum(i, i + 1));
        return answer;
    }
};

Test generate_test(int commands_count, int max_value, int max_elements) {
    Test test;
    Test::Command command;
    int elements_count = 0;
    for (int i = 0; i < commands_count; i++) {
        if (i != 0)
            command.type = static_cast<Test::Command::CommandType>((rand() % 2)?0:3);
        else
            command.type = Test::Command::INSERT;
        command.left = rand() % (elements_count + int(command.type == Test::Command::INSERT || elements_count == 0));
        command.right = rand() % (elements_count - command.left + int(elements_count == 0)) + command.left + 1;
        command.value = rand() % (max_value + 1);
        test.commands.push_back(command);
    }
    return test;
}

Treap treap;
NaiveRope naive_rope;
vector<int> treap_result, naive_result;

int main()
{
    srand(42);
    for (int i = 0; i < 10; i++) {
        Test test = generate_test(1000, 1000, 100);
        treap_result = test.run(&treap);
        naive_result = test.run(&naive_rope);
        if (treap_result != naive_result)
            cout << "Error" << endl;
    }
    return 0;
}
