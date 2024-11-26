#pragma once
#include <memory>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "thread.h"
#include "log.h"
#include "util.h"

namespace arvin
{
    class ConfigVarBase
    {
    public:
        using ptr = std::shared_ptr<ConfigVarBase>;
        ConfigVarBase(const std::string &name, const std::string &decription = "") : m_name(name), m_description(decription)
        {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }
        virtual ~ConfigVarBase();
        const std::string &getName() const { return m_name; }
        const std::string &getDescription() const { return m_description; }
        virtual std::string toString() = 0;
        virtual bool fromString(const std::string &val) = 0;
        virtual std::string getTypeName() const = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };

    template <class F, class T>
    class LexicalCast
    {
    public:
        T operator()(const F &v)
        {
            return boost::lexical_cast<T>(v);
        }
    };

    template <class T>
    class LexicalCast<std::string, std::vector<T>>
    {
    public:
        std::vector<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            using vec = std::vector<T>;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); i++)
            {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };


}