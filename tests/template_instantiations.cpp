/**
 * Creates template instantiations of most classes and functions to make sure they are actually compiled.
 */

#include "copper.h"

namespace copper {

// buffered_channel<int>
template std::optional<int> buffered_channel<int>::try_pop_for(const std::chrono::duration<int>&);
template std::optional<int> buffered_channel<int>::try_pop_until(
    const std::chrono::time_point<std::chrono::system_clock>&);
template bool buffered_channel<int>::push(int&);
template bool buffered_channel<int>::try_push(int&);
template bool buffered_channel<int>::try_push_for(int&, const std::chrono::duration<int>&);
template bool buffered_channel<int>::try_push_until(int&, const std::chrono::time_point<std::chrono::system_clock>&);
template channel_op_status buffered_channel<int>::pop_func(std::function<void(int)>&);
template channel_op_status buffered_channel<int>::try_pop_func(std::function<void(int)>&);
template channel_op_status buffered_channel<int>::try_pop_func_for(std::function<void(int)>&,
                                                                   const std::chrono::duration<int>&);
template channel_op_status buffered_channel<int>::try_pop_func_until(
    std::function<void(int)>&,
    const std::chrono::time_point<std::chrono::system_clock>&);
template channel_op_status buffered_channel<int>::push_func(std::function<int()>&);
template channel_op_status buffered_channel<int>::try_push_func(std::function<int()>&);
template channel_op_status buffered_channel<int>::try_push_func_for(std::function<int()>&,
                                                                    const std::chrono::duration<int>&);
template channel_op_status buffered_channel<int>::try_push_func_until(
    std::function<int()>&,
    const std::chrono::time_point<std::chrono::system_clock>&);

// unbuffered_channel<std::string>
template std::optional<std::string> unbuffered_channel<std::string>::try_pop_for(const std::chrono::duration<int>&);
template std::optional<std::string> unbuffered_channel<std::string>::try_pop_until(
    const std::chrono::time_point<std::chrono::system_clock>&);
template bool unbuffered_channel<std::string>::push(const std::string&);
template bool unbuffered_channel<std::string>::try_push(const std::string&);
template bool unbuffered_channel<std::string>::try_push_for(const std::string&, const std::chrono::duration<int>&);
template bool unbuffered_channel<std::string>::try_push_until(
    const std::string&,
    const std::chrono::time_point<std::chrono::system_clock>&);
template channel_op_status unbuffered_channel<std::string>::pop_func(std::function<void(std::string_view)>&);
template channel_op_status unbuffered_channel<std::string>::try_pop_func(std::function<void(std::string_view)>&);
template channel_op_status unbuffered_channel<std::string>::try_pop_func_for(std::function<void(std::string_view)>&,
                                                                             const std::chrono::duration<int>&);
template channel_op_status unbuffered_channel<std::string>::try_pop_func_until(
    std::function<void(std::string_view)>&,
    const std::chrono::time_point<std::chrono::system_clock>&);
template channel_op_status unbuffered_channel<std::string>::push_func(std::function<std::string()>&);
template channel_op_status unbuffered_channel<std::string>::try_push_func(std::function<std::string()>&);
template channel_op_status unbuffered_channel<std::string>::try_push_func_for(std::function<std::string()>&,
                                                                              const std::chrono::duration<int>&);
template channel_op_status unbuffered_channel<std::string>::try_push_func_until(
    std::function<std::string()>&,
    const std::chrono::time_point<std::chrono::system_clock>&);

// buffered_channel<std::unique_ptr<int>>
template std::optional<std::unique_ptr<int>> buffered_channel<std::unique_ptr<int>>::try_pop_for(
    const std::chrono::duration<int>&);
template std::optional<std::unique_ptr<int>> buffered_channel<std::unique_ptr<int>>::try_pop_until(
    const std::chrono::time_point<std::chrono::system_clock>&);
template bool buffered_channel<std::unique_ptr<int>>::push(std::unique_ptr<int>&&);
template bool buffered_channel<std::unique_ptr<int>>::try_push(std::unique_ptr<int>&&);
template bool buffered_channel<std::unique_ptr<int>>::try_push_for(std::unique_ptr<int>&&,
                                                                   const std::chrono::duration<int>&);
template bool buffered_channel<std::unique_ptr<int>>::try_push_until(
    std::unique_ptr<int>&&,
    const std::chrono::time_point<std::chrono::system_clock>&);
template channel_op_status buffered_channel<std::unique_ptr<int>>::pop_func(std::function<void(std::unique_ptr<int>)>&);
template channel_op_status buffered_channel<std::unique_ptr<int>>::try_pop_func(
    std::function<void(std::unique_ptr<int>)>&);
template channel_op_status buffered_channel<std::unique_ptr<int>>::try_pop_func_for(
    std::function<void(std::unique_ptr<int>)>&,
    const std::chrono::duration<int>&);
template channel_op_status buffered_channel<std::unique_ptr<int>>::try_pop_func_until(
    std::function<void(std::unique_ptr<int>)>&,
    const std::chrono::time_point<std::chrono::system_clock>&);
template channel_op_status buffered_channel<std::unique_ptr<int>>::push_func(std::function<std::unique_ptr<int>()>&);
template channel_op_status buffered_channel<std::unique_ptr<int>>::try_push_func(
    std::function<std::unique_ptr<int>()>&);
template channel_op_status buffered_channel<std::unique_ptr<int>>::try_push_func_for(
    std::function<std::unique_ptr<int>()>&,
    const std::chrono::duration<int>&);
template channel_op_status buffered_channel<std::unique_ptr<int>>::try_push_func_until(
    std::function<std::unique_ptr<int>()>&,
    const std::chrono::time_point<std::chrono::system_clock>&);

// unbuffered_channel<std::unique_ptr<int>>
template std::optional<std::unique_ptr<int>> unbuffered_channel<std::unique_ptr<int>>::try_pop_for(
    const std::chrono::duration<int>&);
template std::optional<std::unique_ptr<int>> unbuffered_channel<std::unique_ptr<int>>::try_pop_until(
    const std::chrono::time_point<std::chrono::system_clock>&);
template bool unbuffered_channel<std::unique_ptr<int>>::push(std::unique_ptr<int>&&);
template bool unbuffered_channel<std::unique_ptr<int>>::try_push(std::unique_ptr<int>&&);
template bool unbuffered_channel<std::unique_ptr<int>>::try_push_for(std::unique_ptr<int>&&,
                                                                     const std::chrono::duration<int>&);
template bool unbuffered_channel<std::unique_ptr<int>>::try_push_until(
    std::unique_ptr<int>&&,
    const std::chrono::time_point<std::chrono::system_clock>&);
template channel_op_status unbuffered_channel<std::unique_ptr<int>>::pop_func(
    std::function<void(std::unique_ptr<int>)>&);
template channel_op_status unbuffered_channel<std::unique_ptr<int>>::try_pop_func(
    std::function<void(std::unique_ptr<int>)>&);
template channel_op_status unbuffered_channel<std::unique_ptr<int>>::try_pop_func_for(
    std::function<void(std::unique_ptr<int>)>&,
    const std::chrono::duration<int>&);
template channel_op_status unbuffered_channel<std::unique_ptr<int>>::try_pop_func_until(
    std::function<void(std::unique_ptr<int>)>&,
    const std::chrono::time_point<std::chrono::system_clock>&);
template channel_op_status unbuffered_channel<std::unique_ptr<int>>::push_func(std::function<std::unique_ptr<int>()>&);
template channel_op_status unbuffered_channel<std::unique_ptr<int>>::try_push_func(
    std::function<std::unique_ptr<int>()>&);
template channel_op_status unbuffered_channel<std::unique_ptr<int>>::try_push_func_for(
    std::function<std::unique_ptr<int>()>&,
    const std::chrono::duration<int>&);
template channel_op_status unbuffered_channel<std::unique_ptr<int>>::try_push_func_until(
    std::function<std::unique_ptr<int>()>&,
    const std::chrono::time_point<std::chrono::system_clock>&);

// buffered_channel<void>
template bool buffered_channel<void>::try_pop_for(const std::chrono::duration<int>&);
template bool buffered_channel<void>::try_pop_until(const std::chrono::time_point<std::chrono::system_clock>&);
template channel_op_status buffered_channel<void>::pop_func(std::function<void()>&);
template channel_op_status buffered_channel<void>::try_pop_func(std::function<void()>&);
template channel_op_status buffered_channel<void>::try_pop_func_for(std::function<void()>&,
                                                                    const std::chrono::duration<int>&);
template channel_op_status buffered_channel<void>::try_pop_func_until(
    std::function<void()>&,
    const std::chrono::time_point<std::chrono::system_clock>&);

template channel_op_status buffered_channel<void>::push_func(std::function<void()>&);
template channel_op_status buffered_channel<void>::try_push_func(std::function<void()>&);
template channel_op_status buffered_channel<void>::try_push_func_for(std::function<void()>&,
                                                                     const std::chrono::duration<int>&);
template channel_op_status buffered_channel<void>::try_push_func_until(
    std::function<void()>&,
    const std::chrono::time_point<std::chrono::system_clock>&);

// unbuffered_channel<void>
template bool unbuffered_channel<void>::try_pop_for(const std::chrono::duration<int>&);
template bool unbuffered_channel<void>::try_pop_until(const std::chrono::time_point<std::chrono::system_clock>&);
template channel_op_status unbuffered_channel<void>::pop_func(std::function<void()>&);
template channel_op_status unbuffered_channel<void>::try_pop_func(std::function<void()>&);
template channel_op_status unbuffered_channel<void>::try_pop_func_for(std::function<void()>&,
                                                                      const std::chrono::duration<int>&);
template channel_op_status unbuffered_channel<void>::try_pop_func_until(
    std::function<void()>&,
    const std::chrono::time_point<std::chrono::system_clock>&);
template channel_op_status unbuffered_channel<void>::push_func(std::function<void()>&);
template channel_op_status unbuffered_channel<void>::try_push_func(std::function<void()>&);
template channel_op_status unbuffered_channel<void>::try_push_func_for(std::function<void()>&,
                                                                       const std::chrono::duration<int>&);
template channel_op_status unbuffered_channel<void>::try_push_func_until(
    std::function<void()>&,
    const std::chrono::time_point<std::chrono::system_clock>&);

// channel_read_view<buffered_channel<int>>
template class channel_read_view<buffered_channel<int>>;
template auto channel_read_view<buffered_channel<int>>::pop_wt<wait_type::for_>(const std::chrono::duration<int>&);
template auto channel_read_view<buffered_channel<int>>::pop_func_wt<wait_type::none>(const std::function<void(int&&)>&);
template auto channel_read_view<buffered_channel<int>>::try_pop_for(const std::chrono::duration<int>&);
template auto channel_read_view<buffered_channel<int>>::try_pop_until(
    const std::chrono::time_point<std::chrono::system_clock>&);
template auto channel_read_view<buffered_channel<int>>::pop_func(std::function<void(int)>&);
template auto channel_read_view<buffered_channel<int>>::try_pop_func(std::function<void(int)>&);
template auto channel_read_view<buffered_channel<int>>::try_pop_func_for(std::function<void(int)>&,
                                                                         const std::chrono::duration<int>&);
template auto channel_read_view<buffered_channel<int>>::try_pop_func_until(
    std::function<void(int)>&,
    const std::chrono::time_point<std::chrono::system_clock>&);

// channel_write_view<unbuffered_channel<int>>
template class channel_write_view<unbuffered_channel<int>>;
template auto channel_write_view<unbuffered_channel<int>>::push_wt<wait_type::none, int>(int&&);
template auto channel_write_view<unbuffered_channel<int>>::push_func_wt<wait_type::for_>(
    const std::function<int()>&,
    const std::chrono::duration<int>&);
template auto channel_write_view<unbuffered_channel<int>>::push(int&);
template auto channel_write_view<unbuffered_channel<int>>::try_push(int&);
template auto channel_write_view<unbuffered_channel<int>>::try_push_for(int&, const std::chrono::duration<int>&);
template auto channel_write_view<unbuffered_channel<int>>::try_push_until(
    int&,
    const std::chrono::time_point<std::chrono::system_clock>&);
template auto channel_write_view<unbuffered_channel<int>>::push_func(const std::function<int()>&);
template auto channel_write_view<unbuffered_channel<int>>::try_push_func(const std::function<int()>&);
template auto channel_write_view<unbuffered_channel<int>>::try_push_func_for(const std::function<int()>&,
                                                                             const std::chrono::duration<int>&);
template auto channel_write_view<unbuffered_channel<int>>::try_push_func_until(
    const std::function<int()>&,
    const std::chrono::time_point<std::chrono::system_clock>&);

// channel_read_view<unbuffered_channel<void>>
template class channel_read_view<unbuffered_channel<void>>;
template auto channel_read_view<buffered_channel<void>>::pop_wt<wait_type::for_>(const std::chrono::duration<int>&);
template auto channel_read_view<buffered_channel<void>>::pop_func_wt<wait_type::none>(const std::function<void()>&);
template auto channel_read_view<unbuffered_channel<void>>::try_pop_for(const std::chrono::duration<int>&);
template auto channel_read_view<unbuffered_channel<void>>::try_pop_until(
    const std::chrono::time_point<std::chrono::system_clock>&);
template auto channel_read_view<buffered_channel<void>>::pop_func(std::function<void()>&);
template auto channel_read_view<buffered_channel<void>>::try_pop_func(std::function<void()>&);
template auto channel_read_view<buffered_channel<void>>::try_pop_func_for(std::function<void()>&,
                                                                          const std::chrono::duration<int>&);
template auto channel_read_view<buffered_channel<void>>::try_pop_func_until(
    std::function<void()>&,
    const std::chrono::time_point<std::chrono::system_clock>&);

// channel_write_view<buffered_channel<void>>
template class channel_write_view<buffered_channel<void>>;
template auto channel_write_view<unbuffered_channel<void>>::push_wt<wait_type::none>();
template auto channel_write_view<unbuffered_channel<void>>::push_func_wt<wait_type::for_>(
    const std::function<void()>&,
    const std::chrono::duration<int>&);
template auto channel_write_view<unbuffered_channel<void>>::push();
template auto channel_write_view<unbuffered_channel<void>>::try_push();
template auto channel_write_view<buffered_channel<void>>::try_push_for(const std::chrono::duration<int>&);
template auto channel_write_view<buffered_channel<void>>::try_push_until(
    const std::chrono::time_point<std::chrono::system_clock>&);
template auto channel_write_view<unbuffered_channel<void>>::push_func(const std::function<void()>&);
template auto channel_write_view<unbuffered_channel<void>>::try_push_func(const std::function<void()>&);
template auto channel_write_view<unbuffered_channel<void>>::try_push_func_for(const std::function<void()>&,
                                                                              const std::chrono::duration<int>&);
template auto channel_write_view<unbuffered_channel<void>>::try_push_func_until(
    const std::function<void()>&,
    const std::chrono::time_point<std::chrono::system_clock>&);

// channel_pop_iterator
template class channel_pop_iterator<buffered_channel<std::unique_ptr<int>>>;
template class channel_pop_iterator<unbuffered_channel<std::unique_ptr<int>>>;
template class channel_pop_iterator<buffered_channel<int>>;
template class channel_pop_iterator<unbuffered_channel<int>>;

// channel_push_iterator
template class channel_push_iterator<buffered_channel<std::unique_ptr<int>>>;
template class channel_push_iterator<unbuffered_channel<std::unique_ptr<int>>>;
template class channel_push_iterator<buffered_channel<int>>;
template class channel_push_iterator<unbuffered_channel<int>>;

// Concept asserts
#if __cpp_concepts >= 201907L
static_assert(std::input_iterator<channel_pop_iterator<buffered_channel<int>>>);
static_assert(std::input_iterator<channel_pop_iterator<unbuffered_channel<int>>>);
static_assert(std::output_iterator<channel_push_iterator<buffered_channel<int>>, int>);
static_assert(std::output_iterator<channel_push_iterator<unbuffered_channel<int>>, int>);
static_assert(std::ranges::range<buffered_channel<int>>);
static_assert(std::ranges::range<unbuffered_channel<int>>);
#endif

}  // namespace copper
