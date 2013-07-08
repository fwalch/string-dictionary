/*
 * Copyright 2010-2011 Chris Vaszauskas and Tyler Richard
 *
 * This file is part of a HAT-trie implementation following the paper
 * entitled "HAT-trie: A Cache-concious Trie-based Data Structure for
 * Strings" by Nikolas Askitis and Ranjan Sinha.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HAT_SET_H
#define HAT_SET_H

#include "hat_trie.hpp"

namespace stx {

template <class T> class hat_set;

/**
 * @brief HAT-trie based set that implements most of the STL set interface
 *
 * Note: the only available template parameter is std::string. Using
 * any other template parameter will result in a compile-time error.
 */
template <>
class hat_set<std::string> {

  private:
    typedef hat_trie<std::string> hat_trie_type;
    typedef hat_set<std::string>  _self;

  public:
    // STL types
    typedef hat_trie_type::size_type         size_type;
    typedef hat_trie_type::key_type          key_type;
    typedef hat_trie_type::value_type        value_type;

    typedef hat_trie_type::iterator          iterator;
    typedef hat_trie_type::const_iterator    const_iterator;

    /**
     * Default constructor.
     *
     * O(1)
     *
     * @param traits     hat trie customization traits
     * @param ah_traits  array hash customization traits
     */
    hat_set(const hat_trie_traits &traits = hat_trie_traits(),
            const array_hash_traits &ah_traits = array_hash_traits()) :
            trie(traits, ah_traits) { }

    /**
     * Array hash traits constructor.
     *
     * @param ah_traits  array hash customization traits
     */
    hat_set(const array_hash_traits &ah_traits) :
            trie(ah_traits) { }

    /**
     * Builds a HAT set from the data in [first, last).
     *
     * O(1)
     *
     * @param first, last  iterators specifying a range of elements to
     *                     initialize the tree with
     */
    template <class input_iterator>
    hat_set(const input_iterator &first, const input_iterator &last,
            const hat_trie_traits &traits = hat_trie_traits(),
            const array_hash_traits &ah_traits = array_hash_traits()) :
        trie(first, last, traits, ah_traits)
    { }

    /**
     * Searches for a word in the trie.
     *
     * O(m)  m = length of the string
     *
     * @param word  word to search for
     * @return  true iff @a word is in the trie
     */
    bool exists(const key_type &word) const {
        return trie.exists(word);
    }

    /**
     * Counts the number of times a word appears in the trie.
     *
     * In set containers, this number will either be 1 or 0.
     *
     * O(m)  m = length of the string
     *
     * @param word  word to search for
     * @return  number of times @a word appears in the trie
     */
    size_type count(const key_type &word) const {
        return trie.count(word);
    }

    /**
     * Determines whether this set is empty.
     *
     * O(1)
     *
     * @return  true iff the container has no data
     */
    bool empty() const {
        return trie.empty();
    }

    /**
     * Gets the number of elements in the trie.
     *
     * O(1)
     *
     * @return  number of elements in the trie
     */
    size_type size() const {
        return trie.size();
    }

    /**
     * Gets a const reference to the traits associated with this trie.
     *
     * O(1)
     *
     * @return  traits associated with this trie
     */
    const hat_trie_traits &traits() const {
        return trie.traits();
    }

    /**
     * Gets the array hash traits associated with the hash tables in
     * this trie.
     *
     * O(1)
     *
     * @return  array hash traits associated with this trie
     */
    const array_hash_traits &hash_traits() const {
        return trie.hash_traits();
    }

    /**
     * Removes all the elements in the trie.
     */
    void clear() {
        trie.clear();
    }

    /**
     * Inserts a word into the trie.
     *
     * According to the standard, this function should return a
     * pair<iterator, bool> rather than just a bool. The lack of a
     * pair returning insert function is intentional because a pair
     * returning insert takes twice as long as the bool version
     * implemented now. The slowdown is significant enough to make
     * insertion time comparable to an STL set (red-black tree), whuch
     * is unacceptable for a high-performance data structure like a
     * HAT-trie. Calling insert() then find() is the intended solution.
     *
     * I consider this acceptable because iterators are unstable. Like
     * STL iterators, HAT-trie iterators are invalidated on any call to
     * insert or erase because memory may be rearranged by a burst
     * operation. Additionally, in a typical scenario, keys aren't operated
     * on as they are inserted. Insertion and operation occur in two
     * different passes, meaning any iterators collected during the
     * insertion pass would be invalidated by the start of the operation
     * pass anyway.
     *
     * Feel free to notify me if you disagree.
     *
     * O(m)  m = length of the string
     *
     * @param word  word to insert
     *
     * @return  true if @a word is inserted into the trie, false if @a word
     *          was already in the trie
     */
    bool insert(const value_type &word) {
        return trie.insert(word);
    }

    /**
     * Inserts a word into the trie.
     *
     * Uses C-strings instead of C++ strings. This function is no faster
     * than the string version. It is provided because calling insert()
     * with a C-string invokes an expensive string copy operation if
     * the string version is the only function provided.
     *
     * O(m)  m = length of the string
     *
     * @param word  word to insert
     * @return  true if @a word is inserted into the trie, false if @a word
     *          was already in the trie
     */
    bool insert(const char *word) {
        return trie.insert(word);
    }

    /**
     * Inserts several words into the trie.
     *
     * O(n)  n = elements in [first, last)
     *
     * @param first, last  iterators specifying a range of words to add
     *                     to the trie. All words in the range
     *                     [first, last) are added
     */
    template <class input_iterator>
    void insert(const input_iterator &first, const input_iterator &last) {
        trie.insert(first, last);
    }

    /**
     * Inserts several words into the trie.
     *
     * In standard STL sets, this function can dramatically increase
     * performance if @a position is set correctly. This performance
     * gain is unachievable in a HAT-trie because the time required to
     * verify that @a position points to the right place is just as
     * expensive as a regular insert operation.
     *
     * O(m)  m = length of the string
     *
     * @param pos   unused
     * @param word  word to insert
     * @return iterator to @a word in the trie
     */
    iterator insert(const iterator &pos, const value_type &word) {
        return trie.insert(pos, word);
    }

    /**
     * Erases a word from the trie.
     *
     * @param word  word to erase
     */
    size_type erase(const key_type &word) {
        return trie.erase(word);
    }

    /**
     * Erases a word from the trie.
     *
     * @param pos  iterator to the word to erase
     */
    void erase(const iterator &pos) {
        trie.erase(pos);
    }

    /**
     * Gets an iterator to the first element in the trie.
     *
     * If there are no elements in the trie, the iterator pointing to
     * trie.end() is returned.
     *
     * O(1)
     *
     * @return  iterator to the first element in the trie
     */
    iterator begin() const {
        return trie.begin();
    }

    /**
     * Gets an iterator to one past the last element in the trie.
     *
     * O(1)
     *
     * @return iterator to one past the last element in the trie
     */
    iterator end() const {
        return trie.end();
    }

    /**
     * Searches for @a word in the trie.
     *
     * O(m)  m = length of the string
     *
     * @param word  word to search for
     * @return  iterator to @a word in the trie, or @a end() if @a word
     *          is not found
     */
    iterator find(const key_type &word) const {
        return trie.find(word);
    }

    /**
     * Swaps the data in two hat_set objects.
     *
     * O(1)
     *
     * @param rhs  hat_set object to swap data with
     */
    void swap(_self &rhs) {
        trie.swap(rhs.trie);
    }

    /**
     * Prints the hierarchical structure of the trie.
     *
     * The output is indented to indicate trie depth. Words are marked
     * by a ~, and containers are marked by a *. For example, a trie with
     * a @a burst_threshold of 2 with the words the, their, there, they're,
     * train, trust, truth, bear, and breath would produce this output:
     *
     *   b *
     *     reath ~
     *     ear ~
     *   t
     *     h
     *      e ~
     *        r *
     *          e ~
     *        y *
     *          `re ~
     *        i *
     *          r ~
     *     r
     *       a *
     *         in ~
     *       u *
     *         st ~
     *         th ~
     *
     * (This isn't exactly right because of the particular bursting
     * algorithm this implementation uses, but it is a good example.)
     *
     * @param out  output stream to print to. cout by default
     */
    void print() {
        trie.print();
    }

    bool operator<(const hat_set<std::string>& rhs) {
        return trie < rhs.trie;
    }

    bool operator<=(const hat_set<std::string>& rhs) {
        return trie <= rhs.trie;
    }

    bool operator>(const hat_set<std::string>& rhs) {
        return trie > rhs.trie;
    }

    bool operator>=(const hat_set<std::string>& rhs) {
        return trie >= rhs.trie;
    }

    bool operator==(const hat_set<std::string>& rhs) {
        return trie == rhs.trie;
    }

    bool operator!=(const hat_set<std::string>& rhs) {
        return trie != rhs.trie;
    }

  private:
    hat_trie_type trie;

};

}  // namespace stx

namespace std {

/**
 * Template overload of std::swap for hat_sets.
 *
 * @param lhs, rhs  hat_set objects to swap
 */
//void swap(stx::hat_set<string> &lhs, stx::hat_set<string> &rhs) {
//    lhs.swap(rhs);
//}

}

#endif

