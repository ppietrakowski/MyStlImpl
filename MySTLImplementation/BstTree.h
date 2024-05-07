#include <string>

#include <iostream>
#include <cstdint>

struct BstNode
{
    std::string Key;
    int32_t Value = 0;

    BstNode* Parent = nullptr;
    BstNode* Left = nullptr;
    BstNode* Right = nullptr;

    ~BstNode() noexcept
    {
        puts("Deleting node");

        delete Right;
        delete Left;
    }

    int32_t* Search(const std::string& Key) const
    {
        if (Key == this->Key)
        {
            return const_cast<int32_t*>(&Value);
        }

        if (Key < this->Key && Left)
        {
            return Left->Search(Key);
        }
        else if (Right)
        {
            return Right->Search(Key);
        }

        return nullptr;
    }

    BstNode* SearchNode(const std::string& Key)
    {
        if (Key == this->Key)
        {
            return this;
        }

        if (Key < this->Key && Left)
        {
            return Left->SearchNode(Key);
        }
        else if (Right)
        {
            return Right->SearchNode(Key);
        }

        return nullptr;
    }


    BstNode* FindInOrderSuccesor()
    {
        BstNode* node = this;

        while (node->Left != nullptr)
        {
            node = node->Left;
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
        delete m_Root;
    }

    void Put(const std::string& key, int32_t value)
    {
        if (!m_Root)
        {
            m_Root = new BstNode();
            m_Root->Key = key;
            m_Root->Value = value;
        }
        else
        {
            BstNode* i = m_Root;

            while (true)
            {
                if (i->Key < key)
                {
                    if (!i->Right)
                    {
                        i->Right = new BstNode{key, value, i};
                        break;
                    }

                    i = i->Right;
                }
                else if (i->Key > key)
                {
                    if (!i->Left)
                    {
                        i->Left = new BstNode{key, value, i};
                        break;
                    }

                    i = i->Left;
                }
                else
                {
                    i->Value = value;
                    return;
                }
            }
        }
    }

    int32_t* Search(const std::string& key)
    {
        return m_Root ? m_Root->Search(key) : nullptr;
    }

    const int32_t* Search(const std::string& key) const
    {
        return m_Root ? m_Root->Search(key) : nullptr;
    }

    bool Remove(const std::string& key)
    {
        if (!m_Root)
        {
            return false;
        }

        if (!m_Root->Right && !m_Root->Left && m_Root->Key == key)
        {
            delete m_Root;
            m_Root = nullptr;
        }

        BstNode* node = m_Root->SearchNode(key);

        if (!node)
        {
            return false;
        }

        BstNode* successor = node->Left;

        if (node->Right)
        {
            successor = node->Right->FindInOrderSuccesor();
        }

        if (successor)
        {
            node->Value = successor->Value;
            node->Key = successor->Key;
        }
        else
        {
            successor = node;
        }

        if (successor->Parent->Left == successor)
        {
            successor->Parent->Left = nullptr;
        }
        else
        {
            successor->Parent->Right = nullptr;
        }

        delete successor;
    }

private:
    BstNode* m_Root = nullptr;
};