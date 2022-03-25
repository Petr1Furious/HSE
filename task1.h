#include <algorithm>
#include <iostream>
#include <vector>

template <class ValueType> class Set {
private:

    struct TreeNode {
        std::vector<TreeNode*> sons;
        ValueType max;
        TreeNode* parent{};

        TreeNode() = default;

        void sort_children() {
            std::sort(sons.begin(), sons.end(), [](const TreeNode* const son1, const TreeNode* const son2){return son1->max < son2->max;});
        }
    };

    TreeNode* root{};
    size_t size_{};

    TreeNode* search(const ValueType& val) const {
        TreeNode* node = root;

        if (node == nullptr) {
            return nullptr;
        }

        while (node->sons.size() != 0) {
            if (node->sons.size() == 2) {
                if (node->sons[0]->max < val) {
                    node = node->sons[1];
                } else {
                    node = node->sons[0];
                }
            } else {
                if (node->sons[1]->max < val) {
                    node = node->sons[2];
                } else if (node->sons[0]->max < val) {
                    node = node->sons[1];
                } else {
                    node = node->sons[0];
                }
            }
        }

        return node;
    }

    void split_parent(TreeNode* node) {
        if (node->sons.size() == 4) {
            auto *new_node = new TreeNode{.sons = {node->sons[2], node->sons[3]},
                    .max = std::max(node->sons[2]->max, node->sons[3]->max),
                    .parent = node->parent};
            new_node->sons[0]->parent = new_node;
            new_node->sons[1]->parent = new_node;
            node->sons.resize(2);
            node->max = std::max(node->sons[0]->max, node->sons[1]->max);
            if (node->parent != nullptr) {
                node->parent->sons.push_back(new_node);
                node->parent->sort_children();
                split_parent(node->parent);
            } else {
                TreeNode *old_root = root;
                root = new TreeNode{.sons = {old_root, new_node}};
                old_root->parent = root;
                new_node->parent = root;
                root->sort_children();
            }
        }
    }

    void update_keys(TreeNode* node) {
        TreeNode* cur = node->parent;
        while (cur != nullptr) {
            cur->max = cur->sons[0]->max;
            for (uint8_t i = 1; i < cur->sons.size(); i++) {
                cur->max = std::max(cur->max, cur->sons[i]->max);
            }
            cur = cur->parent;
        }
    }

    const TreeNode* first_node() const {
        const TreeNode* cur = root;
        if (cur == nullptr) {
            return nullptr;
        }

        while (!cur->sons.empty()) {
            cur = cur->sons[0];
        }
        return cur;
    }

    const TreeNode* last_node() const {
        const TreeNode* cur = root;
        if (cur == nullptr) {
            return nullptr;
        }

        while (!cur->sons.empty()) {
            cur = cur->sons[cur->sons.size() - 1];
        }
        return cur;
    }

    const TreeNode* next_node(const TreeNode* cur) const {
        const TreeNode* push_down{};
        while (true) {
            const TreeNode* parent = cur->parent;
            if (parent == nullptr) {
                return nullptr;
            }

            if (parent->sons[0] == cur) {
                push_down = parent->sons[1];
                break;
            }
            if (parent->sons.size() == 3 && parent->sons[1] == cur) {
                push_down = parent->sons[2];
                break;
            }

            cur = parent;
        }

        while (!push_down->sons.empty()) {
            push_down = push_down->sons[0];
        }
        return push_down;
    }

    const TreeNode* prev_node(const TreeNode* cur) const {
        if (cur == nullptr) {
            return last_node();
        }

        const TreeNode* push_down{};
        while (true) {
            const TreeNode* parent = cur->parent;
            if (parent == nullptr) {
                return nullptr;
            }

            if (parent->sons[parent->sons.size() - 1] == cur) {
                push_down = parent->sons[parent->sons.size() - 2];
                break;
            }
            if (parent->sons.size() == 3 && parent->sons[1] == cur) {
                push_down = parent->sons[0];
                break;
            }

            cur = parent;
        }

        while (!push_down->sons.empty()) {
            push_down = push_down->sons[push_down->sons.size() - 1];
        }
        return push_down;
    }

    void clear_dfs(TreeNode* node) {
        for (TreeNode* son : node->sons) {
            clear_dfs(son);
        }
        delete node;
    }

public:

    class iterator {
        const Set<ValueType>* s;
        const TreeNode* node{};

    public:

        iterator() = default;

        explicit iterator(const Set<ValueType>& set) : s(&set) {}

        iterator(const Set<ValueType>& set, const TreeNode* const node) : s(&set), node(node) {}

        iterator operator++() {
            node = s->next_node(node);
            return *this;
        }

        const iterator operator++(int) {
            iterator old = *this;
            node = s->next_node(node);
            return old;
        }

        iterator operator--() {
            node = s->prev_node(node);
            return *this;
        }

        const iterator operator--(int) {
            iterator old = *this;
            node = s->prev_node(node);
            return old;
        }

        ValueType operator*() const {
            return node->max;
        }

        const ValueType* operator->() const {
            return &(node->max);
        }

        bool operator==(const iterator& other) const {
            return s == other.s && node == other.node;
        }

        bool operator!=(const iterator& other) const {
            return s != other.s || node != other.node;
        }
    };

    Set() = default;

    template <class iterator_>
    Set(iterator_ begin_, iterator_ end_) {
        while (begin_ != end_) {
            insert(*begin_);
            begin_++;
        }
    }

    Set(const std::initializer_list<ValueType>& initializer_list) {
        for (const ValueType& cur : initializer_list) {
            insert(cur);
        }
    }

    Set(const Set& other) {
        if (other.root == root) {
            return;
        }
        for (const ValueType& cur : other) {
            insert(cur);
        }
    }

    void clear() {
        if (root != nullptr) {
            clear_dfs(root);
            root = nullptr;
            size_ = 0;
        }
    }

    Set& operator=(const Set& other) {
        if (other.root == root) {
            return *this;
        }
        clear();
        for (const ValueType& cur : other) {
            insert(cur);
        }
        return *this;
    }

    ~Set() {
        clear();
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    void insert(const ValueType& val) {
        TreeNode* node = search(val);
        if (node != nullptr && !(node->max < val || val < node->max)) {
            return;
        }

        size_++;

        auto* new_node = new TreeNode{.max = {val}};

        if (root == nullptr) {
            root = new_node;
            return;
        }

        if (node->parent == nullptr) {
            TreeNode* other = root;
            root = new TreeNode{.sons = {other, new_node}};
            other->parent = root;
            new_node->parent = root;
            root->sort_children();
        } else {
            TreeNode* parent = node->parent;
            parent->sons.push_back(new_node);
            new_node->parent = parent;
            parent->sort_children();
            update_keys(new_node);
            split_parent(parent);
        }
        update_keys(new_node);
    }

    void erase(const ValueType& val) {
        TreeNode* node = search(val);

        if (node == nullptr || node->max != val) {
            return;
        }

        size_--;

        if (node->parent == nullptr) {
            delete node;
            root = nullptr;
            return;
        }

        while (true) {
            TreeNode* parent = node->parent;
            TreeNode* brother = (parent->sons[0] == node ? parent->sons[1] : parent->sons[0]);
            TreeNode* grandparent = (parent == nullptr ? nullptr : parent->parent);
            TreeNode* uncle = nullptr;
            if (grandparent != nullptr) {
                if (grandparent->sons[1] == parent) {
                    if (grandparent->sons.size() == 3 && grandparent->sons[2]->sons.size() == 3) {
                        uncle = grandparent->sons[2];
                    } else {
                        uncle = grandparent->sons[0];
                    }
                }
                else {
                    uncle = grandparent->sons[1];
                }
            }

            if (parent != nullptr) {
                if (parent->sons.size() == 3) {
                    parent->sons.erase(std::find(parent->sons.begin(), parent->sons.end(), node));
                    delete node;
                    update_keys(brother);
                    break;
                } else {
                    delete node;
                    if (grandparent == nullptr) {
                        delete parent;
                        root = brother;
                        root->parent = nullptr;
                        break;
                    } else {
                        if (uncle->sons.size() == 2) {
                            uncle->sons.push_back(brother);
                            brother->parent = uncle;
                            uncle->sort_children();
                            update_keys(brother);
                            node = parent;
                        } else {
                            grandparent->sons.erase(std::find(grandparent->sons.begin(), grandparent->sons.end(), parent));
                            delete parent;
                            uncle->sons.push_back(brother);
                            brother->parent = uncle;
                            uncle->sort_children();
                            update_keys(brother);
                            split_parent(uncle);
                            break;
                        }
                    }
                }
            }
        }
    }

    iterator begin() const {
        return iterator(*this, first_node());
    }

    iterator end() const {
        return iterator(*this, nullptr);
    }

    iterator lower_bound(const ValueType& val) const {
        if (root == nullptr) {
            return end();
        }
        TreeNode* cur = root;
        while (!cur->sons.empty()) {
            for (uint8_t i = 0; i < cur->sons.size(); i++) {
                if (!(cur->sons[i]->max < val) || i == cur->sons.size() - 1) {
                    cur = cur->sons[i];
                    break;
                }
            }
        }

        return !(cur->max < val) ? iterator(*this, cur) : end();
    }

    iterator find(const ValueType& val) const {
        iterator it = lower_bound(val);
        if (it == end()) {
            return end();
        }
        return (*it < val || val < *it) ? end() : it;
    }
};
