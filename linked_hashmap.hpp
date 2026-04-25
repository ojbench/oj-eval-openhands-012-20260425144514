/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is the order in which keys were inserted into the map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */
    
template<
	class Key,
	class T,
	class Hash = std::hash<Key>, 
	class Equal = std::equal_to<Key>
> class linked_hashmap {
public:
	typedef pair<const Key, T> value_type;

private:
    struct Node {
        value_type *data;
        Node *prev, *next; // insertion order
        Node *hash_next;   // hash bucket

        Node(const value_type &v) : prev(nullptr), next(nullptr), hash_next(nullptr) {
            data = reinterpret_cast<value_type*>(new char[sizeof(value_type)]);
            new (data) value_type(v);
        }
        Node() : data(nullptr), prev(nullptr), next(nullptr), hash_next(nullptr) {}
        ~Node() {
            if (data) {
                data->~value_type();
                delete[] reinterpret_cast<char*>(data);
            }
        }
    };

    Node **table;
    size_t capacity;
    size_t _size;
    Node *head, *tail; // dummy nodes for insertion order
    Hash hash_func;
    Equal equal_func;

    static const size_t DEFAULT_CAPACITY = 16;
    static constexpr double LOAD_FACTOR = 0.75;

    void resize() {
        size_t new_capacity = capacity * 2;
        Node **new_table = new Node*[new_capacity];
        for (size_t i = 0; i < new_capacity; ++i) new_table[i] = nullptr;

        for (size_t i = 0; i < capacity; ++i) {
            Node *curr = table[i];
            while (curr) {
                Node *next_node = curr->hash_next;
                size_t new_idx = hash_func(curr->data->first) % new_capacity;
                curr->hash_next = new_table[new_idx];
                new_table[new_idx] = curr;
                curr = next_node;
            }
        }
        delete[] table;
        table = new_table;
        capacity = new_capacity;
    }

public:
 
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = linked_hashmap.begin(); --it;
	 *       or it = linked_hashmap.end(); ++end();
	 */
	class const_iterator;
	class iterator {
	private:
        Node *ptr;
        const linked_hashmap *container;
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::bidirectional_iterator_tag;


		iterator(Node *p = nullptr, const linked_hashmap *c = nullptr) : ptr(p), container(c) {}
		iterator(const iterator &other) : ptr(other.ptr), container(other.container) {}

		iterator operator++(int) {
            if (ptr == container->tail) throw invalid_iterator();
            iterator old = *this;
            ptr = ptr->next;
            return old;
        }
		iterator & operator++() {
            if (ptr == container->tail) throw invalid_iterator();
            ptr = ptr->next;
            return *this;
        }
		iterator operator--(int) {
            if (ptr == container->head->next) throw invalid_iterator();
            iterator old = *this;
            ptr = ptr->prev;
            return old;
        }
		iterator & operator--() {
            if (ptr == container->head->next) throw invalid_iterator();
            ptr = ptr->prev;
            return *this;
        }
		value_type & operator*() const {
            if (ptr == nullptr || ptr == container->head || ptr == container->tail) throw invalid_iterator();
            return *(ptr->data);
        }
		bool operator==(const iterator &rhs) const {
            return ptr == rhs.ptr;
        }
		bool operator==(const const_iterator &rhs) const {
            return ptr == rhs.ptr;
        }
		bool operator!=(const iterator &rhs) const {
            return ptr != rhs.ptr;
        }
		bool operator!=(const const_iterator &rhs) const {
            return ptr != rhs.ptr;
        }

		value_type* operator->() const noexcept {
            if (ptr == nullptr || ptr == container->head || ptr == container->tail) return nullptr;
            return ptr->data;
        }

        friend class linked_hashmap;
        friend class const_iterator;
	};
 
	class const_iterator {
	private:
        const Node *ptr;
        const linked_hashmap *container;
	public:
        using difference_type = std::ptrdiff_t;
        using value_type = typename linked_hashmap::value_type;
        using pointer = const value_type*;
        using reference = const value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

		const_iterator(const Node *p = nullptr, const linked_hashmap *c = nullptr) : ptr(p), container(c) {}
		const_iterator(const const_iterator &other) : ptr(other.ptr), container(other.container) {}
		const_iterator(const iterator &other) : ptr(other.ptr), container(other.container) {}

        const_iterator operator++(int) {
            if (ptr == container->tail) throw invalid_iterator();
            const_iterator old = *this;
            ptr = ptr->next;
            return old;
        }
        const_iterator & operator++() {
            if (ptr == container->tail) throw invalid_iterator();
            ptr = ptr->next;
            return *this;
        }
        const_iterator operator--(int) {
            if (ptr == container->head->next) throw invalid_iterator();
            const_iterator old = *this;
            ptr = ptr->prev;
            return old;
        }
        const_iterator & operator--() {
            if (ptr == container->head->next) throw invalid_iterator();
            ptr = ptr->prev;
            return *this;
        }
        const value_type & operator*() const {
            if (ptr == nullptr || ptr == container->head || ptr == container->tail) throw invalid_iterator();
            return *(ptr->data);
        }
        bool operator==(const iterator &rhs) const {
            return ptr == rhs.ptr;
        }
        bool operator==(const const_iterator &rhs) const {
            return ptr == rhs.ptr;
        }
        bool operator!=(const iterator &rhs) const {
            return ptr != rhs.ptr;
        }
        bool operator!=(const const_iterator &rhs) const {
            return ptr != rhs.ptr;
        }

        const value_type* operator->() const noexcept {
            if (ptr == nullptr || ptr == container->head || ptr == container->tail) return nullptr;
            return ptr->data;
        }

        friend class linked_hashmap;
        friend class iterator;
	};
 
	linked_hashmap() : capacity(DEFAULT_CAPACITY), _size(0) {
        table = new Node*[capacity];
        for (size_t i = 0; i < capacity; ++i) table[i] = nullptr;
        head = new Node();
        tail = new Node();
        head->next = tail;
        tail->prev = head;
    }
	linked_hashmap(const linked_hashmap &other) : capacity(other.capacity), _size(0) {
        table = new Node*[capacity];
        for (size_t i = 0; i < capacity; ++i) table[i] = nullptr;
        head = new Node();
        tail = new Node();
        head->next = tail;
        tail->prev = head;
        for (auto it = other.cbegin(); it != other.cend(); ++it) {
            insert(*it);
        }
    }
 
	/**
	 * TODO assignment operator
	 */
	linked_hashmap & operator=(const linked_hashmap &other) {
        if (this == &other) return *this;
        clear();
        delete[] table;
        capacity = other.capacity;
        table = new Node*[capacity];
        for (size_t i = 0; i < capacity; ++i) table[i] = nullptr;
        for (auto it = other.cbegin(); it != other.cend(); ++it) {
            insert(*it);
        }
        return *this;
    }
 
	/**
	 * TODO Destructors
	 */
	~linked_hashmap() {
        clear();
        delete head;
        delete tail;
        delete[] table;
    }
 
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {
        iterator it = find(key);
        if (it == end()) throw index_out_of_bound();
        return it.ptr->data->second;
    }
	const T & at(const Key &key) const {
        const_iterator it = find(key);
        if (it == cend()) throw index_out_of_bound();
        return it.ptr->data->second;
    }
 
	/**
	 * TODO
	 * access specified element 
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
        iterator it = find(key);
        if (it != end()) return it.ptr->data->second;
        return insert(value_type(key, T())).first.ptr->data->second;
    }
 
	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
        return at(key);
    }
 
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
        return iterator(head->next, this);
    }
	const_iterator cbegin() const {
        return const_iterator(head->next, this);
    }
 
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {
        return iterator(tail, this);
    }
	const_iterator cend() const {
        return const_iterator(tail, this);
    }
 
	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {
        return _size == 0;
    }
 
	/**
	 * returns the number of elements.
	 */
	size_t size() const {
        return _size;
    }
 
	/**
	 * clears the contents
	 */
	void clear() {
        Node *curr = head->next;
        while (curr != tail) {
            Node *next_node = curr->next;
            delete curr;
            curr = next_node;
        }
        head->next = tail;
        tail->prev = head;
        for (size_t i = 0; i < capacity; ++i) table[i] = nullptr;
        _size = 0;
    }
 
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion), 
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
        size_t idx = hash_func(value.first) % capacity;
        Node *curr = table[idx];
        while (curr) {
            if (equal_func(curr->data->first, value.first)) {
                return pair<iterator, bool>(iterator(curr, this), false);
            }
            curr = curr->hash_next;
        }

        if (_size + 1 > capacity * LOAD_FACTOR) {
            resize();
            idx = hash_func(value.first) % capacity;
        }

        Node *new_node = new Node(value);
        new_node->hash_next = table[idx];
        table[idx] = new_node;

        new_node->prev = tail->prev;
        new_node->next = tail;
        tail->prev->next = new_node;
        tail->prev = new_node;

        _size++;
        return pair<iterator, bool>(iterator(new_node, this), true);
    }
 
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
        if (pos == end() || pos.container != this || pos.ptr == head || pos.ptr == tail) throw invalid_iterator();
        
        Node *node = pos.ptr;
        size_t idx = hash_func(node->data->first) % capacity;
        Node *curr = table[idx];
        Node *prev_hash = nullptr;
        while (curr) {
            if (curr == node) {
                if (prev_hash) prev_hash->hash_next = curr->hash_next;
                else table[idx] = curr->hash_next;
                break;
            }
            prev_hash = curr;
            curr = curr->hash_next;
        }

        node->prev->next = node->next;
        node->next->prev = node->prev;
        delete node;
        _size--;
    }
 
	/**
	 * Returns the number of elements with key 
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0 
	 *     since this container does not allow duplicates.
	 */
	size_t count(const Key &key) const {
        if (find(key) == cend()) return 0;
        return 1;
    }
 
	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {
        size_t idx = hash_func(key) % capacity;
        Node *curr = table[idx];
        while (curr) {
            if (equal_func(curr->data->first, key)) {
                return iterator(curr, this);
            }
            curr = curr->hash_next;
        }
        return end();
    }
	const_iterator find(const Key &key) const {
        size_t idx = hash_func(key) % capacity;
        Node *curr = table[idx];
        while (curr) {
            if (equal_func(curr->data->first, key)) {
                return const_iterator(curr, this);
            }
            curr = curr->hash_next;
        }
        return cend();
    }
};

}

#endif
