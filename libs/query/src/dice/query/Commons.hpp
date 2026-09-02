#ifndef QUERY_COMMONS_HPP
#define QUERY_COMMONS_HPP

#include <sstream>

#include <dice/hypertrie/Hypertrie.hpp>

namespace dice::query {

	struct query_timeout : std::runtime_error {
	private:
		std::chrono::steady_clock::duration timeout_duration_;

		inline static std::string make_error_msg(std::chrono::steady_clock::duration timeout_duration) {
			std::ostringstream oss;
			oss << "Query evaluation timed out after " << std::chrono::duration_cast<std::chrono::seconds>(timeout_duration).count() << " seconds";
			return oss.str();
		}

	public:
		inline explicit query_timeout(std::chrono::steady_clock::duration timeout_duration) : std::runtime_error{make_error_msg(timeout_duration)},
																							  timeout_duration_{timeout_duration} {
		}

		inline std::chrono::steady_clock::duration timeout_duration() const noexcept {
			return timeout_duration_;
		}
	};

	/**
	 * Stores information that needs to be accessed during query evaluation
	 */
	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct EvaluationContext {
		EvaluationContext() = delete;

		explicit EvaluationContext(hypertrie::const_Hypertrie<htt_t, allocator_type> tensor)
			: tensor(std::move(tensor)),
			  start_time_(std::chrono::system_clock::now()),
			  end_time_(std::chrono::steady_clock::time_point::max()) {}

		EvaluationContext(hypertrie::const_Hypertrie<htt_t, allocator_type> tensor, std::chrono::steady_clock::time_point end_time)
			: tensor(std::move(tensor)),
			  start_time_(std::chrono::system_clock::now()),
			  end_time_(end_time),
			  has_time_out_(end_time < std::chrono::steady_clock::time_point::max()) {
			if (has_time_out_)
				time_out_duration_ = end_time_ - std::chrono::steady_clock::now();
		}

		hypertrie::const_Hypertrie<htt_t, allocator_type> const tensor;

	private:
		// currently, used only for SPARQL's NOW() function
		std::chrono::system_clock::time_point const start_time_;
		std::chrono::steady_clock::time_point const end_time_;
		std::chrono::steady_clock::duration time_out_duration_;
		bool has_time_out_ = false;
		static constexpr uint16_t max_time_out_counter_ = 512;
		mutable uint16_t time_out_counter_ = 0;

	public:
		void check_time_out() const {
			if (has_time_out_ and time_out_counter_++ > max_time_out_counter_) {
				if (std::chrono::steady_clock::now() < end_time_) [[likely]] {
					time_out_counter_ = 0;
				} else {
					throw query_timeout{time_out_duration_};
				}
			}
		}

		std::chrono::system_clock::time_point get_start_time() const {
			return start_time_;
		}

	};

	enum class Operation {
		NoOp,
		Join,
		LeftJoin,
		Union,
		Cartesian,
		Assignment,
		Filter,
		FilterAlt,
		Resolve,
		Count,
		EntryGenerator
	};

	namespace detail {
		using namespace dice::hypertrie;
		template<typename new_value_type,
				 typename key_part_type_o,
				 typename value_type_o,
				 template<typename, typename, typename, typename, typename> class map_type_o,
				 template<typename, typename, typename, typename> class set_type_o,
				 bool taggable_key_part_v_o>
		constexpr auto inject_value_type(Hypertrie_t<key_part_type_o, value_type_o, map_type_o, set_type_o, taggable_key_part_v_o>) {
			return Hypertrie_t<key_part_type_o, new_value_type, map_type_o, set_type_o, taggable_key_part_v_o>{};
		}


	}// namespace detail

	template<typename value_type, hypertrie::HypertrieTrait htt_t>
	using tri_with_value_type = decltype(detail::inject_value_type<value_type>(htt_t{}));

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t>
	using Entry = ::dice::hypertrie::Entry<tri_with_value_type<value_type, htt_t>>;

	template<typename value_type, hypertrie::HypertrieTrait_bool_valued htt_t>
	using Key = ::dice::hypertrie::Key<tri_with_value_type<value_type, htt_t>>;

	using count_type = unsigned long;

	template<hypertrie::HypertrieTrait_bool_valued htt_t>
	using CountedEntry = Entry<count_type, htt_t>;

	template<hypertrie::HypertrieTrait_bool_valued htt_t>
	using CountedKey = Key<count_type, htt_t>;

	using operand_desc = uint8_t;

}// namespace dice::query
#endif//QUERY_COMMONS_HPP
