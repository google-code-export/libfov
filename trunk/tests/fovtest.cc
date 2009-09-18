/*
 * Copyright (C) 2006, Greg McIntyre
 * All rights reserved. See the file named COPYING in the distribution
 * for more details.
 */

#include <algorithm>
#include <cstring>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <ostream>
#include <sstream>
#include <vector>
#include <fov/fov.h>
#define BOOST_TEST_MODULE fovtest
#include <boost/test/included/unit_test.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/multi_array.hpp>

using namespace std;
using namespace boost::assign;
using namespace boost::tuples;

// -------------------------------------------------

namespace std {
    template<class A1, class A2>
    ostream& operator<<(ostream& s, vector<A1, A2> const& vec) {
        copy(vec.begin(), vec.end(), ostream_iterator<A1>(s, "\n"));
        return s;
    }
}

// -------------------------------------------------

struct CountMap {
    CountMap(vector<string> counts);
    CountMap(unsigned w, unsigned h): w(w), h(h), counts(w*h, 0) { }
    void resize(unsigned w, unsigned h);
    bool operator==(const CountMap& b) const { return w == b.w && h == b.h && counts == b.counts; }
    unsigned value(unsigned x, unsigned y) const { return counts[y*w+x]; }
    void increment(unsigned x, unsigned y) { ++counts[y*w+x]; }

    unsigned w;
    unsigned h;
    vector<int> counts;
};

CountMap::CountMap(vector<string> counts): 
    w(counts[0].size()), h(counts.size()), counts(w*h, 0) {
    this->counts.resize(w*h, 0);
    for (unsigned j = 0; j < h; ++j) {
        for (unsigned i = 0; i < w; ++i) {
            this->counts[j*w+i] = counts[h - 1 - j][i] - '0'; // humans work upside down
        }
    }
}

std::ostream& operator<<(std::ostream& out, const CountMap& map) {
    for (int j = (int)map.h - 1; j >= 0; --j) {
        for (int i = 0; i < (int)map.w; ++i) {
            char str[2] = { ('0' + map.value(i, j)), '\0' };
            out << str;
        }
        out << std::endl;
    }
    return out;
}
// -------------------------------------------------

struct Cell {
    unsigned char tile;
    bool seen;

    Cell(void) : tile('.'), seen(false) {}
    void apply(void) { seen = true; }
    bool is_opaque(void) const { return tile == '#'; }

    friend class Map;
};

// -------------------------------------------------

struct Map {
    Map(const vector<string>& raster);
    Map(const Map& map);
    void apply(unsigned x, unsigned y);
    bool is_on_map(unsigned x, unsigned y){ return x < w && y < h; }
    bool is_opaque(unsigned x, unsigned y);

    unsigned w;
    unsigned h;
    vector<Cell> cells;
    CountMap opaque_count_map;
    CountMap apply_count_map;
};

Map::Map(const vector<string>& raster):
    w(raster[0].size()),
    h(raster.size()),
    cells(w*h),
    opaque_count_map(w, h),
    apply_count_map(w, h)
{
    for (unsigned i = 0; i < w; ++i) {
        for (unsigned j = 0; j < h; ++j) {
            Cell& c = cells[j*w + i];
            c.tile = raster[h - 1 - j][i];
        }
    }
}

Map::Map(const Map& map): 
    w(map.w), 
    h(map.h),
    cells(map.cells),
    opaque_count_map(map.opaque_count_map),
    apply_count_map(map.apply_count_map) {
}

bool Map::is_opaque(unsigned x, unsigned y) {
    return cells[y*w+x].is_opaque();
}

void Map::apply(unsigned x, unsigned y) {
    cells[y*w+x].apply();
}

ostream& operator<<(ostream& out, const Map& map) {
    for (int j = (int)map.h - 1; j >= 0; --j) {
        for (int i = 0; i < (int)map.w; ++i) {
            char str[2] = { map.cells[j*map.w+i].tile, '\0' };
            out << str;
        }
        out << endl;
    }
    return out;
}

// -------------------------------------------------

static void apply_increment(void *map, int x, int y, int dx, int dy, void *src) {
    Map *m = static_cast<Map *>(map);
    if (!m->is_on_map(x, y))
        return;
    m->apply_count_map.increment(x, y);
    m->apply(x, y);
}

static bool opaque_increment(void *map, int x, int y) {
    Map *m = static_cast<Map *>(map);
    if (!m->is_on_map(x, y))
        return true;
    m->opaque_count_map.increment(x, y);
    return m->is_opaque(x, y);
}

// -------------------------------------------------

typedef boost::tuple<Map, CountMap, CountMap> BasicCase;

void test_count_maps(Map map, 
        CountMap expected_opaque, 
        CountMap expected_apply, 
        int px, int py, unsigned radius, 
        fov_shape_type shape) {
    fov_settings_type settings;
    fov_settings_init(&settings);
    fov_settings_set_opacity_test_function(&settings, opaque_increment);
    fov_settings_set_apply_lighting_function(&settings, apply_increment);
    fov_settings_set_shape(&settings, shape);
    fov_circle(&settings, &map, NULL, px, py, radius);
    fov_settings_free(&settings);
    BOOST_CHECK(map.opaque_count_map == expected_opaque);
    BOOST_CHECK(map.apply_count_map == expected_apply);
}

void test_count_maps_beam(Map map, 
        CountMap expected_opaque, 
        CountMap expected_apply, 
        int px, int py, unsigned radius, 
        fov_shape_type shape,
        fov_direction_type direction,
        float angle) {
    fov_settings_type settings;
    fov_settings_init(&settings);
    fov_settings_set_opacity_test_function(&settings, opaque_increment);
    fov_settings_set_apply_lighting_function(&settings, apply_increment);
    fov_settings_set_shape(&settings, shape);
    fov_beam(&settings, &map, NULL, px, py, radius, direction, angle);
    fov_settings_free(&settings);
    BOOST_CHECK(map.opaque_count_map == expected_opaque);
    BOOST_CHECK(map.apply_count_map == expected_apply);
}

BOOST_AUTO_TEST_SUITE(test_suite1)

    BOOST_AUTO_TEST_CASE(basics) {
        vector<BasicCase> cases;

        vector<string> raster1 = list_of
            ("..........")
            ("..........")
            ("..........")
            ("..........")
            ("..........")
            ("....@.....")
            ("..........")
            ("..........")
            ("..........")
            ("..........");
        vector<string> expected_apply1 = list_of
            ("0000000000")
            ("0000000000")
            ("0111111100")
            ("0111111100")
            ("0111111100")
            ("0111011100")
            ("0111111100")
            ("0111111100")
            ("0111111100")
            ("0000000000");
        vector<string> expected_opaque1 = list_of
            ("0000000000")
            ("0000000000")
            ("0111211100")
            ("0111211100")
            ("0111211100")
            ("0222022200")
            ("0111211100")
            ("0111211100")
            ("0111211100")
            ("0000000000");
        cases += make_tuple(Map(raster1), CountMap(expected_opaque1), CountMap(expected_apply1));

        vector<string> raster2 = list_of
            ("..........")
            ("..........")
            ("..........")
            ("..........")
            ("...###....")
            ("...#@#....")
            ("...###....")
            ("..........")
            ("..........")
            ("..........");
        vector<string> expected_apply2 = list_of
            ("0000000000")
            ("0000000000")
            ("0000000000")
            ("0000000000")
            ("0001110000")
            ("0001010000")
            ("0001110000")
            ("0000000000")
            ("0000000000")
            ("0000000000");
        vector<string> expected_opaque2 = list_of
            ("0000000000")
            ("0000000000")
            ("0000000000")
            ("0000000000")
            ("0001210000")
            ("0002020000")
            ("0001210000")
            ("0000000000")
            ("0000000000")
            ("0000000000");
        cases += make_tuple(Map(raster2), CountMap(expected_opaque2), CountMap(expected_apply2));

        vector<string> raster3 = list_of
            ("..........")
            ("..........")
            ("..........")
            (".....#####")
            ("##########")
            ("....@.....")
            ("..........")
            ("..........")
            ("..........")
            ("..........");
        vector<string> expected_apply3 = list_of
            ("0000000000")
            ("0000000000")
            ("0000000000")
            ("0000000000")
            ("0111111100")
            ("0111011100")
            ("0111111100")
            ("0111111100")
            ("0111111100")
            ("0000000000");
        vector<string> expected_opaque3 = list_of
            ("0000000000")
            ("0000000000")
            ("0000000000")
            ("0000000000")
            ("0111211100")
            ("0222022200")
            ("0111211100")
            ("0111211100")
            ("0111211100")
            ("0000000000");

        vector<string> raster4 = list_of
            ("..........")
            ("..........")
            ("..........")
            ("..........")
            ("..........")
            ("....@####.")
            ("......###.")
            ("..........")
            ("..........")
            ("..........");
        vector<string> expected_apply4 = list_of
            ("0000000000")
            ("0000000000")
            ("0111111100")
            ("0111111000")
            ("0111110000")
            ("0111010000")
            ("0111110000")
            ("0111111000")
            ("0111111100")
            ("0000000000");
        vector<string> expected_opaque4 = list_of
            ("0000000000")
            ("0000000000")
            ("0111211100")
            ("0111211000")
            ("0111210000")
            ("0222020000")
            ("0111210000")
            ("0111211000")
            ("0111211100")
            ("0000000000");
        const int px = 4;
        const int py = 4;
        const unsigned radius = 3;
        fov_settings_type settings;
        fov_settings_init(&settings);
        fov_settings_set_opacity_test_function(&settings, opaque_increment);
        fov_settings_set_apply_lighting_function(&settings, apply_increment);
        fov_settings_set_shape(&settings, FOV_SHAPE_SQUARE);
        BOOST_FOREACH(BasicCase c, cases) {
            Map &m = get<0>(c);
            fov_circle(&settings, &m, NULL, px, py, radius);
            BOOST_CHECK(m.opaque_count_map == get<1>(c));
            BOOST_CHECK(m.apply_count_map == get<2>(c));
        }
        fov_settings_free(&settings);
    }

    BOOST_AUTO_TEST_CASE(circle) {
        vector<string> raster = list_of
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            (".......@.......")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............");
        vector<string> expected_apply = list_of
            ("000000000000000")
            ("000000000000000")
            ("000011111110000")
            ("000111111111000")
            ("001111111111100")
            ("001111111111100")
            ("001111111111100")
            ("001111101111100")
            ("001111111111100")
            ("001111111111100")
            ("001111111111100")
            ("000111111111000")
            ("000011111110000")
            ("000000000000000")
            ("000000000000000");
        vector<string> expected_opaque = list_of
            ("000000000000000")
            ("000000000000000")
            ("000011121110000")
            ("000111121111000")
            ("001111121111100")
            ("001111121111100")
            ("001111121111100")
            ("002222202222200")
            ("001111121111100")
            ("001111121111100")
            ("001111121111100")
            ("000111121111000")
            ("000011121110000")
            ("000000000000000")
            ("000000000000000");
        const int px = 7;
        const int py = 7;
        const unsigned radius = 6;
        test_count_maps(Map(raster), CountMap(expected_opaque), CountMap(expected_apply), px, py, radius, FOV_SHAPE_CIRCLE);
    }

    BOOST_AUTO_TEST_CASE(octagon) {
        vector<string> raster = list_of
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            (".......@.......")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............");
        vector<string> expected_apply = list_of
            ("000000000000000")
            ("000000000000000")
            ("000001111100000")
            ("000111111111000")
            ("000111111111000")
            ("001111111111100")
            ("001111111111100")
            ("001111101111100")
            ("001111111111100")
            ("001111111111100")
            ("000111111111000")
            ("000111111111000")
            ("000001111100000")
            ("000000000000000")
            ("000000000000000");
        vector<string> expected_opaque = list_of
            ("000000000000000")
            ("000000000000000")
            ("000001121100000")
            ("000111121111000")
            ("000111121111000")
            ("001111121111100")
            ("001111121111100")
            ("002222202222200")
            ("001111121111100")
            ("001111121111100")
            ("000111121111000")
            ("000111121111000")
            ("000001121100000")
            ("000000000000000")
            ("000000000000000");
        const int px = 7;
        const int py = 7;
        const unsigned radius = 6;
        test_count_maps(Map(raster), CountMap(expected_opaque), CountMap(expected_apply), px, py, radius, FOV_SHAPE_OCTAGON);
    }

    BOOST_AUTO_TEST_CASE(wall_face) {
        vector<string> raster = list_of
            ("..............................")
            ("##############################")
            ("@.............................")
            ("..............................");
        vector<string> expected_apply = list_of
            ("000000000000000000000000000000")
            ("111111111111111111111111111111")
            ("011111111111111111111111111111")
            ("111111111111111111111111111111");
        vector<string> expected_opaque = list_of
            ("000000000000000000000000000000")
            ("211111111111111111111111111111")
            ("022222222222222222222222222222")
            ("211111111111111111111111111111");
        const int px = 0;
        const int py = 1;
        const unsigned radius = 40;
        test_count_maps(Map(raster), CountMap(expected_opaque), CountMap(expected_apply), px, py, radius, FOV_SHAPE_SQUARE);
    }

    BOOST_AUTO_TEST_CASE(beam) {
        vector<string> raster = list_of
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("@..............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............");
        vector<string> expected_apply = list_of
            ("000000000000011")
            ("000000000001111")
            ("000000000111111")
            ("000000011111111")
            ("000001111111111")
            ("000111111111111")
            ("011111111111111")
            ("011111111111111")
            ("011111111111111")
            ("000111111111111")
            ("000001111111111")
            ("000000011111111")
            ("000000000111111")
            ("000000000001111")
            ("000000000000011");
        vector<string> expected_opaque = list_of
            ("000000000000011")
            ("000000000001111")
            ("000000000111111")
            ("000000011111111")
            ("000001111111111")
            ("000111111111111")
            ("011111111111111")
            ("022222222222222")
            ("011111111111111")
            ("000111111111111")
            ("000001111111111")
            ("000000011111111")
            ("000000000111111")
            ("000000000001111")
            ("000000000000011");
        const int px = 0;
        const int py = 7;
        const unsigned radius = 20;
        const fov_direction_type direction = FOV_EAST;
        const float angle = 45.0f;
        test_count_maps_beam(Map(raster), CountMap(expected_opaque), CountMap(expected_apply), px, py, radius, FOV_SHAPE_SQUARE, direction, angle);
    }

    BOOST_AUTO_TEST_CASE(grow) {
        vector<string> raster = list_of
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("@..............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............");
        vector<string> expected_apply = list_of
            ("000000000000011")
            ("000000000001111")
            ("000000000111111")
            ("000000011111111")
            ("000001111111111")
            ("000111111111111")
            ("011111111111111")
            ("011111111111111")
            ("011111111111111")
            ("000111111111111")
            ("000001111111111")
            ("000000011111111")
            ("000000000111111")
            ("000000000001111")
            ("000000000000011");
        vector<string> expected_opaque = list_of
            ("000000000000011")
            ("000000000001111")
            ("000000000111111")
            ("000000011111111")
            ("000001111111111")
            ("000111111111111")
            ("011111111111111")
            ("022222222222222")
            ("011111111111111")
            ("000111111111111")
            ("000001111111111")
            ("000000011111111")
            ("000000000111111")
            ("000000000001111")
            ("000000000000011");
        const int px = 0;
        const int py = 7;
        int radius;
        const fov_direction_type direction = FOV_EAST;
        const float angle = 45.0f;
        Map map(raster);
        CountMap expected_opaque_count_map(expected_opaque);
        CountMap expected_apply_count_map(expected_apply);
        fov_settings_type settings;
        fov_settings_init(&settings);
        fov_settings_set_opacity_test_function(&settings, opaque_increment);
        fov_settings_set_apply_lighting_function(&settings, apply_increment);
        fov_settings_set_shape(&settings, FOV_SHAPE_SQUARE);

        radius = 20;
        fov_beam(&settings, &map, NULL, px, py, radius, direction, angle);
        BOOST_CHECK(map.opaque_count_map == expected_opaque_count_map);
        BOOST_CHECK(map.apply_count_map == expected_apply_count_map);

        // Again with radius++, to cause re-allocation of the heights array.
        radius = 20000;
        map = Map(raster); // reset count maps
        fov_beam(&settings, &map, NULL, px, py, radius, direction, angle);
        BOOST_CHECK(map.opaque_count_map == expected_opaque_count_map);
        BOOST_CHECK(map.apply_count_map == expected_apply_count_map);

        fov_settings_free(&settings);
    }

    BOOST_AUTO_TEST_CASE(beam_behind_orthogonal) {
        vector<string> raster = list_of
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("@......#.......")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............")
            ("...............");
        vector<string> expected_apply = list_of
            ("000000000000011")
            ("000000000001111")
            ("000000000111111")
            ("000000011111111")
            ("000001111111111")
            ("000111111111111")
            ("011111111111111")
            ("011111110000000")
            ("011111111111111")
            ("000111111111111")
            ("000001111111111")
            ("000000011111111")
            ("000000000111111")
            ("000000000001111")
            ("000000000000011");
        vector<string> expected_opaque = list_of
            ("000000000000011")
            ("000000000001111")
            ("000000000111111")
            ("000000011111111")
            ("000001111111111")
            ("000111111111111")
            ("011111111111111")
            ("022222220000000")
            ("011111111111111")
            ("000111111111111")
            ("000001111111111")
            ("000000011111111")
            ("000000000111111")
            ("000000000001111")
            ("000000000000011");
        const unsigned radius = 20;
        const int px = 0;
        const int py = 7;
        const fov_direction_type direction = FOV_EAST;
        const float angle = 45.0f;
        test_count_maps_beam(Map(raster), CountMap(expected_opaque), CountMap(expected_apply), px, py, radius, FOV_SHAPE_SQUARE, direction, angle);
    }


BOOST_AUTO_TEST_SUITE_END()
