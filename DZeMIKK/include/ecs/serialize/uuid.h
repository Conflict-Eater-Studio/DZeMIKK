#pragma once
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <nlohmann/json.hpp>

namespace nlohmann {
template <> struct adl_serializer<boost::uuids::uuid> {
    static void to_json(json& j, const boost::uuids::uuid& u) {
        j = boost::uuids::to_string(u);
    }

    static void from_json(const json& j, boost::uuids::uuid& u) {
        static boost::uuids::string_generator gen;
        u = gen(j.get<std::string>());
    }
};
}