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

#ifndef _SRC_CCollectionHash_hpp__
#define _SRC_CCollectionHash_hpp__

#include <map>
#include <string>
#include <mutex>
#include <boost/filesystem.hpp>

#include "CompareFolders.hpp"

namespace fs = boost::filesystem;

namespace  cf {
    
    /// @brief This is a collection of hashes
    /// @details Internally it is built on two symetrical maps that can give the hash of a file or
    /// the files producing a given hash (useful if some files are duplicated).
    /// All operations are **not** thread safe!
    class CCollectionHash
    {
    public:
        /// @brief Constructor from a given path
        /// @param root Root folder containing the hashed files
        CCollectionHash(const fs::path& root) noexcept :
            _root{ root }
        {   }
        /// @brief Constructor from a given JSON file
        /// @param json Collection in the JSON format
        /// @detailed Constructs the collection from a JSON formatted string describing its content.
        CCollectionHash(const std::string& json) noexcept;

        ~CCollectionHash() = default;
        
        /// @brief Adds a hash corresponding to a given path
        void setHash(const fs::path& path, const std::string& hash);

        /// @brief Exports the hashes as a JSON string
        std::string json() const;
        
        /// @brief Removes the path from the collection
        void removePath(const fs::path& path);
        
        /// @brief Compares the collection to another one.
        diff_t compare(CCollectionHash rhs) const;
        
        /// @brief Converts the stored paths and their hashes to a string
        std::string toString() const;

        /// @brief returns the number of paths
        inline unsigned size() {
            return _file_hashes.size();
        }
        
    private:
        std::map<fs::path, std::string> _file_hashes;           ///< File pathes and their corresponding hash
        std::map<std::string, std::list<fs::path>> _hash_files; ///< Hash with the corresponding files. Useful for duplicate files.
        const fs::path _root;                                   ///< Root folder containing all the files hashed 
    };
    
}


#endif /* _SRC_CCollectionHash_hpp__ */
