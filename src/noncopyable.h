#pragma once

namespace arvin
{
  class Noncopyable
  {
  public:
    /**
     * @brief 默认构造函数
     */
    Noncopyable() = default;

    /**
     * @brief 默认析构函数
     */
    ~Noncopyable() = default;

    /**
     * @brief 拷贝构造函数(禁用)
     */
    Noncopyable(const Noncopyable &) = delete;

    /**
     * @brief 赋值函数(禁用)
     */
    Noncopyable &operator=(const Noncopyable &) = delete;
  };

}