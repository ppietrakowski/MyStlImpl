#include <string>

#include <iostream>
#include <cstdint>

struct BstNode
{
    std::string key;
    int32_t value = 0;

    BstNode* parent = nullptr;
    BstNode* left = nullptr;
    BstNode* right = nullptr;

    ~BstNode() noexcept
    {
        puts("Deleting node");

        delete right;
        delete left;
    }

    int32_t* Search(const std::string& Key) const
    {
        if (Key == this->key)
        {
            return const_cast<int32_t*>(&value);
        }

        if (Key < this->key && left)
        {
            return left->Search(Key);
        }
        else if (right)
        {
            return right->Search(Key);
        }

        return nullptr;
    }

    BstNode* SearchNode(const std::string& Key)
    {
        if (Key == this->key)
        {
            return this;
        }

        if (Key < this->key && left)
        {
            return left->SearchNode(Key);
        }
        else if (right)
        {
            return right->SearchNode(Key);
        }

        return nullptr;
    }


    BstNode* FindInOrderSuccesor()
    {
        BstNode* node = this;

        while (node->left != nullptr)
        {
            node = node->left;
        }

        return node;
    }
};

class BinarySearchTree
{
public:
    BinarySearchTree() = default;
    ~BinarySearchTree() noexcept
    {
        delete root;
    }

    void Put(const std::string& key, int32_t value)
    {
        if (!root)
        {
            root = new BstNode();
            root->key = key;
            root->value = value;
        }
        else
        {
            BstNode* i = root;

            while (true)
            {
                if (i->key < key)
                {
                    if (!i->right)
                    {
                        i->right = new BstNode{key, value, i};
                        break;
                    }

                    i = i->right;
                }
                else if (i->key > key)
                {
                    if (!i->left)
                    {
                        i->left = new BstNode{key, value, i};
                        break;
                    }

                    i = i->left;
                }
                else
                {
                    i->value = value;
                    return;
                }
            }
        }
    }

    int32_t* Search(const std::string& key)
    {
        return root ? root->Search(key) : nullptr;
    }

    const int32_t* Search(const std::string& key) const
    {
        return root ? root->Search(key) : nullptr;
    }

    bool Remove(const std::string& key)
    {
        if (!root)
        {
            return false;
        }

        if (!root->right && !root->left && root->key == key)
        {
            delete root;
            root = nullptr;
        }

        BstNode* node = root->SearchNode(key);

        if (!node)
        {
            return false;
        }

        BstNode* successor = node->left;

        if (node->right)
        {
            successor = node->right->FindInOrderSuccesor();
        }

        if (successor)
        {
            node->value = successor->value;
            node->key = successor->key;
        }
        else
        {
            successor = node;
        }

        if (successor->parent->left == successor)
        {
            successor->parent->left = nullptr;
        }
        else
        {
            successor->parent->right = nullptr;
        }

        delete successor;
    }

private:
    BstNode* root = nullptr;
};