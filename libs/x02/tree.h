#pragma once

#include <exception>
#include <memory> // unique_ptr
#include <string>
#include <string_view>
#include <vector>

#include "../common.h"

class Tree;
class Node;

namespace detail {

// Constructor of Node must be private-like to satisfy parent-child invariants.
// We use the passkey idiom to give access to both Tree and Node, as well
// as enabling make_unique(args) instead of unique_ptr(new Node(args)).
//
struct NodeCreateKey {
private:
    friend Tree;
    friend Node;
    NodeCreateKey() = default;

    static std::unique_ptr<Node> create(Tree& tree, Node* parent, std::string_view name) {
        NodeCreateKey key;
        return std::make_unique<Node>(key, tree, parent, name);
    }
};

} // namespace detail

class API Node {
public:
    Node(detail::NodeCreateKey, Tree& tree, Node* parent, std::string_view name)
        : tree_(tree)
        , parent_(parent)
        , name_(name) {
    }

    // Ideally, we'd also want to implement ~Node() too to avoid
    // stack overflow when destructing vector<unique_ptr<Node>>, see:
    // https://www.youtube.com/watch?v=JfmTagWcqoE&t=1012s

    // Cannot be copied or moved because tree/nodes store the addresses of
    // nodes. Would otherwise require custom copy/move implementations.
    //
    DISABLE_COPY_AND_MOVE(Node);

    // guaranteed non-null by invariant
    Tree& tree() const {
        return tree_;
    }

    // could be nullptr, e.g., the root.
    // Note: Using pointer since std::optional<Node&> is not a thing.
    //       https://www.foonathan.net/2018/07/optional-reference/
    Node* parent() const {
        return parent_;
    }

    std::string_view name() const {
        return name_;
    }

    void setName(std::string_view name) {
        name_ = name;
    }

    size_t numChildren() const {
        return children_.size();
    }

    // guaranteed non-null if it exists, otherwise throws
    Node& child(size_t index) const {
        return *children_.at(index);
    }

    // guaranteed non-null, throws if memory allocation fails
    Node& createChild(std::string_view name) {
        children_.push_back(detail::NodeCreateKey::create(tree(), this, name));
        return *children_.back();
    }

    void clearChildren() {
        children_.clear();
    }

private:
    Tree& tree_; // we assume nodes cannot change trees
    Node* parent_ = nullptr;
    std::vector<std::unique_ptr<Node>> children_;
    std::string name_;
};

class API Tree {
public:
    // Cannot be copied or moved because nodes store the addrees to the tree.
    // Would otherwise require custom copy/move implementations.
    //
    DISABLE_COPY_AND_MOVE(Tree);

    Tree()
        : root_(detail::NodeCreateKey::create(*this, nullptr, "root")) {
    }

    // guaranteed non-null: our tree is assumed to always has a root.
    Node& root() {
        return *root_;
    };

private:
    std::unique_ptr<Node> root_;
};

// Side questions about constness:
//
// - Should `Node::firstChild() const` return a const Node*?
//
// - But would this require to have both `const Node* child() const` and
//   `Node* child()` versions?
//
// - Is it worth it to bother, given that Python bindings will ignore constness
//   anyway?
//
// - Same question about Node::tree()? Tree::root()?
//
// Fundamentally, this is a question of desired semantics:
//
// - Should a modification of a child node be considered as a modification of
//   the parent node too?
//
// - Surely, it is a modification of the tree though, isn't it? (e.g.,
//   modifying an element in a std::list<T> is considered as a modification of
//   the list itself)
//
// - But isn't the "tree" the same as the "root node of the tree", if the node
//   themselves owns their child nodes via unique_ptr?
//
// - Does it make sense to have a difference in constness-behavior between
//   vector<Node> and vector<unique_ptr<Node>>, when the reason we use the
//   unique_ptr is not to change the semantics, but because of technical reasons?
//   (faster to move when re-allocating the vector, etc.)
//
