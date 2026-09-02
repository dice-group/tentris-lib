#ifndef TENTRIS_QUERY_NODEWRAPPER_HPP
#define TENTRIS_QUERY_NODEWRAPPER_HPP

#include <dice/hash/DiceHash.hpp>
#include <rdf4cpp/rdf.hpp>
#include <dice/hypertrie/internal/raw/node_context/common_detail/Container.hpp>

namespace dice::node_wrapper {
using namespace rdf4cpp::rdf;

class NodeWrapper : public Node {
protected:
    explicit NodeWrapper(NodeBackendHandle id) noexcept : Node(id) {}

public:
    NodeWrapper() noexcept = default;

	NodeWrapper(Node node) noexcept : Node(node) {
	}

	explicit NodeWrapper(uint64_t const raw) noexcept : Node{NodeBackendHandle::from_raw(raw)} {
	}

	bool operator==(const NodeWrapper &other) const noexcept {
        return this->backend_handle().raw() == other.backend_handle().raw();
    }

	bool operator!=(const NodeWrapper &other) const noexcept {
        return this->backend_handle().raw() != other.backend_handle().raw();
    }

	auto operator<=>(const NodeWrapper &other) const noexcept {
		return static_cast<Node const &>(*this) <=> static_cast<Node const &>(other);
    }

	explicit operator uint64_t() const noexcept {
		return this->backend_handle().raw();
	}

    explicit operator bool() const noexcept {
        return bool(ebv());
    }
};
};  // namespace dice::node_wrapper

template<typename Policy>
struct dice::hash::dice_hash_overload<Policy, dice::node_wrapper::NodeWrapper> {
    static std::size_t dice_hash(dice::node_wrapper::NodeWrapper const &x) noexcept {
        return Policy::hash_fundamental(x.backend_handle().raw());
    }
};

template<>
struct std::hash<dice::node_wrapper::NodeWrapper> {
    size_t operator()(dice::node_wrapper::NodeWrapper const &x) const noexcept {
        return x.backend_handle().raw();
    }
};

namespace dice::hypertrie::internal::raw::node_context::common_detail {
	template<>
	struct Hash<::dice::node_wrapper::NodeWrapper> {
		using is_avalanching = void;

		size_t operator()(::dice::node_wrapper::NodeWrapper const wrap) const noexcept {
			return ::dice::hash::Policies::wyhash::hash_fundamental(wrap.backend_handle().raw());
		}
	};
} // namespace dice::hypertrie::internal::raw::node_context::common_detail

#endif  //TENTRIS_QUERY_NODEWRAPPER_HPP
