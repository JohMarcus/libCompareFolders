/*
3  *  Copyright (C) 2017 Christophe Meneboeuf <christophe@xtof.info>
4  *
5  *  This program is free software: you can redistribute it and/or modify
6  *  it under the terms of the GNU General Public License as published by
7  *  the Free Software Foundation, either version 3 of the License, or
8  *  (at your option) any later version.
9  *
10  *  This program is distributed in the hope that it will be useful,
11  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
12  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
13  *  GNU General Public License for more details.
14  *
15  *  You should have received a copy of the GNU General Public License
16  *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
17 */

#include <list>
#include <vector>
#include <future>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>

#include "CompareFolders.hpp"

#include "CFactoryHashes.hpp"


#include <iostream>

using namespace std;
namespace pt = boost::property_tree;

namespace cf {
    
    ////////////////////////
    
    /// @detailed   Computing hashes can take some time. Thus, an external error logger must be provided
    ///             to give the opportunity to report the errors in real time.
    CCollectionHash CFactoryHashes::computeHashes(const fs::path& root, ILogError& logger) const
    {
        const auto paths = listFiles(root);
        CCollectionHash hashes{ root };

        try 
        {
            for (const auto& path : paths)
            {
                constexpr bool isUpperCase = true;
                string hash;
                CryptoPP::SHA1 hasher;
                CryptoPP::FileSource(path.c_str(), true, \
                    new CryptoPP::HashFilter(hasher, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), isUpperCase))
                );
                const auto path_relative = fs::relative(path, root);
                const auto time_modified = fs::last_write_time(path);

                hashes.setHash(path_relative, { hash, time_modified } );
            }
        }
        catch (const CryptoPP::Exception& e) {
            const string message = string{ "Hashing error: " } +e.what();
            logger.log(message);
        }
      
        return hashes;
    }


    CCollectionHash CFactoryHashes::readHashes(const fs::path& json_path) const
    {
        if(!fs::is_regular_file(json_path)) {
            throw ExceptionFatal{json_path.string() + " is not a file."};
        }
        pt::ptree root;
        pt::read_json(json_path.string(), root);
        const auto generator= root.get<string>(JSON_KEYS.GENERATOR);
        if (generator != JSON_CONST_VALUES.GENERATOR) {
            throw ExceptionFatal{ "This is not a proper file." };
        }
        CCollectionHash collection{ fs::path{ root.get<string>(JSON_KEYS.ROOT) }};

        for (const auto& file : root.get_child(JSON_KEYS.CONTENT.FILES)) {
            const auto hash = file.second.get_child(JSON_KEYS.CONTENT.HASH).data();
            const time_t time = std::stoll(file.second.get_child(JSON_KEYS.CONTENT.TIME).data());
            collection.setHash(fs::path{ file.first }, { hash, time });
        }
        return collection;
    }
    

    ///////////////////////

    const std::list<fs::path> CFactoryHashes::listFiles(const fs::path& dir) const
    {
        // Collect the files list
        std::list<fs::path> paths;
        try
        {
            for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                if(fs::is_regular_file(entry.path())) {
                    paths.push_back(entry.path());
                }
            }
        }
        catch (const fs::filesystem_error& e) {
            throw(ExceptionFatal{ e.what() });
        }
        return paths;
    }
    
    
}
