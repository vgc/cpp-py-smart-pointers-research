#pragma once

#include <exception>
#include <memory> // shared_ptr
#include <string>
#include <string_view>
#include <vector>

#include "../common.h"

class Tree;
class Node;

using NodeSharedPtr = std::shared_ptr<Node>;
using NodeWeakPtr = std::weak_ptr<Node>;

namespace detail {

// Constructor of Node must be private-like to enforce that it is created via
// make_shared. We use the passkey idiom to give access to both Tree and Node.
//
struct NodeCreateKey {
private:
    friend Tree;
    friend Node;
    NodeCreateKey() = default;

    static NodeSharedPtr create(Tree* tree, Node* parent, std::string_view name) {
        NodeCreateKey key;
        return std::make_shared<Node>(key, tree, parent, name);
    }
};

} // namespace detail

class API Node : public std::enable_shared_from_this<Node> {
public:
    // Note: we cannot just write parent_(parent) since weak_ptr doesn't have
    // a constructor from a raw pointer, and we cannot write
    // parent->weak_from_this() directly since parent might be nullptr.
    //
    Node(detail::NodeCreateKey, Tree* tree, Node* parent, std::string_view name)
        : tree_(tree)
        , parent_(parent ? parent->weak_from_this() : NodeWeakPtr())
        , name_(name) {
    }

    // As opposed to x02, it is now mandatory to implement ~Node() in order to
    // ensure that tree_ and parent_ of children are set to nullptr, in order
    // to keep the same semantics as x02: the whole subtree is deleted, even if
    // some external observers still have shared pointers to the node or any of
    // its descendant. In other words, even though memory lifetime is shared,
    // there is still a unique object responsible for the parent-child
    // relationships: the tree itself.
    //
    ~Node() {
        clearChildren();
    }

    // Cannot be copied or moved because tree/nodes store the addresses of
    // nodes. Would otherwise require custom copy/move implementations.
    //
    DISABLE_COPY_AND_MOVE(Node);

    // Cannot guarantee to be non-null by invariant, because the node is
    // managed by shared ptr but the tree is not, and there is no way for the
    // node to keep alive the tree. So the node may outlive its tree, in which
    // case its `tree()` is set to `nullptr`.
    //
    // Problem: callers of this function cannot "lock" the returned value.
    // Hence, this cannot be made thread-safe, as opposed to the `parent()`
    // whose usage can be made thread-safe.
    //
    Tree* tree() const {
        return tree_;
    }

    // Null in case we're the root or we've been removed from the tree.
    NodeWeakPtr parent() const {
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

    // Guaranteed non-null if it exists, otherwise throws.
    // Might be deleted from another thread by the time you call `lock()` though.
    NodeWeakPtr child(size_t index) const {
        return children_.at(index);
    }

    // Guaranteed non-null, throws if memory allocation fails.
    // Might be deleted from another thread by the time you call `lock()` though.
    NodeWeakPtr createChild(std::string_view name) {
        children_.push_back(detail::NodeCreateKey::create(tree(), this, name));
        return children_.back();
    }

    void clearChildren() {
        for (auto child : children_) {
            child->detach_();
        }
        children_.clear();
    }

private:
    Tree* tree_;
    NodeWeakPtr parent_;
    std::vector<NodeSharedPtr> children_;
    std::string name_;

    // Note: Node::shared_from_this() cannot be called from the destructor of
    // Node (bad_weak_ptr exception). This is why in ~Node(), we call this for
    // each children (which we know are still alive C++ objects since we still
    // own a non-null shared_ptr to them), but not on the current node itself.
    //
    friend Tree;
    void detach_() {
        std::vector<NodeSharedPtr> stack; // allows non-recursive implementation
        stack.push_back(shared_from_this());
        while (!stack.empty()) {
            NodeSharedPtr node = stack.back();
            stack.pop_back();
            for (const auto& child : node->children_) {
                stack.push_back(child);
            }
            node->tree_ = nullptr;
            node->parent_.reset();
            node->children_.clear();
        }
    }
};

class API Tree {
public:
    // Cannot be copied or moved because nodes store the addrees to the tree.
    // Would otherwise require custom copy/move implementations.
    //
    DISABLE_COPY_AND_MOVE(Tree);

    Tree()
        : root_(detail::NodeCreateKey::create(this, nullptr, "root")) {
    }

    ~Tree() {
        // Note: no need to pin here: Tree has no mutator of root_ In fact,
        // `root_` could be made a `NonNullSharedPtr`: a custom shared_ptr that
        // does not have a reset_() function and throws if constructed with a
        // nullptr.
        root_->detach_();
    }

    // guaranteed non-null: our tree is assumed to always has a root.
    NodeWeakPtr root() {
        return root_;
    };

private:
    NodeSharedPtr root_;
};

// Note: same questions about constness as in x02.
// We could have SharedConstPtr and WeakConstPtr.
