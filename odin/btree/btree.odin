package btree

import "core:builtin"
import "core:intrinsics"

Elem :: int
Order :: 4

lower_bound_proc :: proc(vals : []$E, v : E, less : proc(E, E) -> bool) -> int {
	n := builtin.len(vals) 
	lo, hi := 0, n
	for lo != hi {
		assert(lo < hi)
		mid := lo + (hi - lo) / 2
		assert(lo <= mid && mid < hi)
		if less(vals[mid], v) {
			lo = mid + 1
		} else {
			hi = mid
		}
	}
	return lo
}

lower_bound_val :: proc(vals : []$E, v : E) -> int where intrinsics.type_is_comparable(E) {
	less :: proc(e0, e1 : E) -> bool { return e0 < e1 }
	return lower_bound_proc(vals, v, less)
}

lower_bound :: proc{ lower_bound_proc, lower_bound_val }

upper_bound_proc :: proc(vals : []$E, v : E, less : proc(E, E) -> bool) -> int {
	n := builtin.len(vals) 
	lo, hi := 0, n
	for lo != hi {
		assert(lo < hi)
		mid := lo + (hi - lo) / 2
		assert(lo <= mid && mid < hi)
		if less(v, vals[mid]) {
			hi = mid
		} else {
			lo = mid + 1
		}
	}
	return lo
}

upper_bound_val :: proc(vals : []$E, v : E) -> int where intrinsics.type_is_comparable(E) {
	less :: proc(e0, e1 : E) -> bool { return e0 < e1 }
	return upper_bound_proc(vals, v, less)
}

upper_bound :: proc{ upper_bound_proc, upper_bound_val }

Node :: struct {
	parent : ^Node,
	numValues : int,
	children : ^[Order]^Node,
	values : [Order - 1]Elem,
}

NodeWithChildren :: struct {
	node : Node,
	childArray : [Order]^Node,
}

alloc_node :: proc(leaf : bool) -> ^Node {
	if leaf {
		return new(Node)
	} else {
		withChildren := new (NodeWithChildren)
		withChildren^.node.children = &withChildren^.childArray
		assert(rawptr(withChildren) == &withChildren^.node)
		return &withChildren^.node
	}
}

free_node :: proc(node : ^Node) {
	free(node)
}

free_node_with_children :: proc(node : ^Node) {
	if node == nil do return
	if node^.children != nil {
		for child in node^.children[:node^.numValues + 1] {
			free_node_with_children(child)
		}
	}
	free_node(node)
}

set_child :: proc(node : ^Node, childIndex : int, child : ^Node) {
	node^.children[childIndex] = child
	child^.parent = node
}

set_children :: proc(node : ^Node, index : int, children : []^Node) {
	for i in 0..<builtin.len(children) {
		set_child(node, index + i, children[i])
	}
}

parent_node :: proc(node : ^Node) -> (parent : ^Node, childIndex : int) {
	parent = node^.parent
	if parent == nil do return
	childIndex = lower_bound(parent^.values[:node^.numValues], node^.values[0])
	for childIndex < parent^.numValues + 1 && parent^.children[childIndex] != node {
		childIndex += 1
	}
	assert(childIndex < parent^.numValues + 1)
	assert(parent^.children[childIndex] == node)
	return
}

leftmost_descendant :: proc(node : ^Node) -> ^Node {
	if node == nil do return nil
	node := node
	for node.children != nil {
		assert(node.numValues > 0)
		node = node.children[0]
	}
	return node
}

rightmost_descendant :: proc(node : ^Node) -> ^Node {
	if node == nil do return nil
	node := node
	for node.children != nil {
		assert(node.numValues > 0)
		node = node.children[node.numValues];
	}
	return node
}

split_full_node :: proc(node : ^Node, extra : Elem, extraNode : ^Node, extraIndex : Elem) -> (middle : Elem, newNode : ^Node) {
	assert(node^.numValues == Order - 1)
	assert(0 <= extraIndex && extraIndex <= node^.numValues)
	assert((node^.children != nil) == (extraNode != nil))
	newNode = alloc_node(node^.children == nil)
	newNode^.numValues = (Order - 1) / 2
	node^.numValues -= newNode^.numValues

	switch {
	case extraIndex < node^.numValues:
		// extra value is in the original (left) node
		middle = node^.values[node^.numValues-1]
		copy(newNode^.values[:newNode^.numValues], node^.values[node^.numValues:])
		copy(node^.values[extraIndex+1:node^.numValues], node^.values[extraIndex:node^.numValues-1])
		node^.values[extraIndex] = extra
		if extraNode != nil {
			set_children(newNode, 0, node^.children[node^.numValues:])
			copy(node^.children[extraIndex+2:node^.numValues+1], node^.children[extraIndex+1:node^.numValues])
			set_child(node, extraIndex + 1, extraNode)
		}
	case node^.numValues < extraIndex:
		// extra value is in the new (right) node
		middle = node^.values[node^.numValues]
		extraIndexNew := extraIndex - node^.numValues
		copy(newNode^.values[:extraIndexNew], node^.values[node^.numValues+1:extraIndex])
		newNode^.values[extraIndexNew] = extra
		copy(newNode^.values[extraIndexNew+1:newNode^.numValues], node^.values[extraIndex:])
		if extraNode != nil {
			set_children(newNode, 0, node^.children[node^.numValues+1:extraIndex+1])
			set_child(newNode, extraIndexNew + 1, extraNode)
			set_children(newNode, extraIndexNew + 2, node^.children[extraIndex+1:])
		}
	case:
		// extra value is in the middle between the nodes
		middle = extra
		copy(newNode^.values[:newNode^.numValues], node^.values[node^.numValues:])
		if extraNode != nil {
			set_child(newNode, 0, extraNode)
			set_children(newNode, 1, node^.children[node^.numValues+1:])
		}
	}
	return middle, newNode
}

Iterator :: struct {
	node : ^Node,
	valueIndex : int,
}

next :: proc(it : Iterator) -> Iterator {
	assert(it.valueIndex < it.node^.numValues)
	if it.node^.children != nil {
		// first index in next child to the right
		return Iterator{it.node^.children[it.valueIndex + 1], 0}
	}
	if it.valueIndex < Order - 2 {
		// next index in the same leaf
		return Iterator{it.node, it.valueIndex + 1}
	}
	for node := it.node; true; {
		// go up the tree and point to the next value, if there's one
		parent, childIndex := parent_node(node)
		if parent == nil { 
			break
		}
		if childIndex < parent^.numValues {
			// we've reached a parent that has a next value after the last if the values of it
			return Iterator{parent, childIndex}
		}
		node = parent
	}
	// no more parents up, we've reached the end
	return Iterator{it.node, it.node^.numValues}
}

prev :: proc(it : Iterator) -> Iterator {
	assert(it.valueIndex <= it.node^.numValues)
	if it.node^.children != nil {
		// last index of left child
		child := it.node^.children[it.valueIndex]
		return Iterator{child, child.numValues - 1}
	}
	if it.valueIndex > 0 {
		// previous index in the same leaf
		return Iterator{it.node, it.valueIndex - 1}
	}
	for node := it.node; true; {
		// go up the tree and point to the prev value, if there's one
		parent, childIndex := parent_node(node)
		if parent == nil { 
			// no more parents up, we've trying to decrement begin()
			assert(false)
			break
		}
		if childIndex > 0 {
			// we've reached a parent that has a prev value after the last if the values of it
			return Iterator{parent, childIndex - 1}
		}
		node = parent
	}
	// return the same iterator
	return it
}

valid :: proc "contextless" (it : Iterator) -> bool {
	return it.node != nil && 0 <= it.valueIndex && it.valueIndex < it.node^.numValues
}

get :: proc(it : Iterator) -> (Elem, bool) {
	if !valid(it) {
		return {}, false
	}
	return it.node^.values[it.valueIndex], true
}

Btree :: struct {
	root : ^Node,
	numValues : int,
}

init :: proc(tree : ^Btree) {
	assert(tree^.root == nil)
	assert(tree^.numValues == 0)
}

destroy :: proc(tree : ^Btree) {
	clear(tree)
}

len :: proc(tree : ^Btree) -> int {
	return tree^.numValues
}

clear :: proc(tree : ^Btree) {
	free_node_with_children(tree^.root)
	tree^.root = nil
	tree^.numValues = 0
}

check_integrity :: proc(tree : ^Btree) -> bool {
	return false
}

begin :: proc(tree : ^Btree) -> Iterator {
	return Iterator{leftmost_descendant(tree^.root), 0}
}

end :: proc(tree : ^Btree) -> Iterator {
	node := rightmost_descendant(tree^.root)
	return Iterator{node, node^.numValues if node != nil else 0}
}

get_bound_tree :: proc(tree : ^Btree, v : Elem, $bound : proc([]Elem, Elem) -> int) -> Iterator {
	if tree^.root == nil do return Iterator{}
	node := tree^.root
	for {
		boundIndex := bound(node^.values[:node^.numValues], v)
		if node^.children == nil {
			return Iterator{node, boundIndex}
		}
		assert(0 <= boundIndex && boundIndex <= node^.numValues)
		node = node^.children[boundIndex]
	}
}

lower_bound_tree :: proc(tree : ^Btree, v : Elem) -> Iterator {
	lower_bound_elem :: proc(vals : []Elem, v : Elem) -> int {
		return lower_bound(vals, v)
	}
	return get_bound_tree(tree, v, lower_bound_elem)
}

upper_bound_tree :: proc(tree : ^Btree, v : Elem) -> Iterator {
	upper_bound_elem :: proc(vals : []Elem, v : Elem) -> int {
		return upper_bound(vals, v)
	}
	return get_bound_tree(tree, v, upper_bound_elem)
}

find :: proc(tree : ^Btree, v : Elem) -> Iterator {
	it := lower_bound_tree(tree, v)
	if !valid(it) {
		return it
	}
	return end(tree)
}

insert_at :: proc(tree : ^Btree, v : Elem, vNode : ^Node, it : Iterator) -> Iterator {
	assert(it.node != nil)
	assert((vNode != nil) == (it.node^.children != nil))
	if it.node^.numValues < Order - 1 {
		// there's space to insert the new node here
		copy(it.node^.values[it.valueIndex+1:it.node^.numValues+1], it.node^.values[it.valueIndex:it.node^.numValues])
		it.node^.values[it.valueIndex] = v
		if vNode != nil {
			copy(it.node^.children[it.valueIndex+2:it.node^.numValues+2], it.node^.children[it.valueIndex+1:it.node^.numValues+1])
			set_child(it.node, it.valueIndex + 1, vNode)
		}
		it.node^.numValues += 1
		tree^.numValues += 1
		return it
	}

	// node is full, we split it and insert the middle element in the parent node
	middle, middleNode := split_full_node(it.node, v, vNode, it.valueIndex)
	parent, indexInParent := parent_node(it.node)
	if parent == nil {
		// grow the tree, create a new root and attach the current root as its only child
		assert(it.node == tree^.root)
		tree^.root = alloc_node(false)
		set_child(tree^.root, 0, it.node)
		parent = tree^.root
		indexInParent = 0
	}
	// tail recursion, should be equivalent to a loop
	return insert_at(tree, middle, middleNode, Iterator{parent, indexInParent})
}

insert :: proc(tree : ^Btree, v : Elem) -> Iterator {
	if tree^.root == nil {
		tree^.root = alloc_node(true)
	}
	it := upper_bound_tree(tree, v)
	return insert_at(tree, v, nil, it)
}

erase :: proc(tree : ^Btree, it : Iterator) {
	assert(0 <= it.valueIndex && it.valueIndex < it.node^.numValues)
	it := it
	if it.node^.children != nil {
		left := leftmost_descendant(it.node)
		right := rightmost_descendant(it.node)
		newIt : Iterator
		if right^.numValues > left^.numValues {
			newIt = Iterator{right, right.numValues - 1}
		} else {
			newIt = Iterator{left, 0}
		}
		it.node^.values[it.valueIndex] = newIt.node^.values[newIt.valueIndex]
		it = newIt
	}
	assert(it.node^.children == nil)

	remove_element :: proc(it : Iterator) {
		copy(it.node^.values[it.valueIndex:it.node^.numValues-1], it.node^.values[it.valueIndex + 1:it.node^.numValues])
		if it.node^.children != nil {
			copy(it.node^.children[it.valueIndex+1:it.node^.numValues], it.node^.children[it.valueIndex+2:it.node^.numValues+1])
		}
		it.node^.numValues -= 1
	}

	minValues :: Order / 2 - 1

	assert(0 <= it.valueIndex && it.valueIndex < it.node^.numValues)
	if it.node^.numValues > minValues || it.node == tree^.root {
		// just remove the element from the node, it'll remain valid
		remove_element(it)
		if it.node == tree^.root && it.node^.numValues == 0 {
			if it.node^.children != nil {
				tree^.root = it.node^.children[0]
				tree^.root.parent = nil
			} else {
				tree^.root = nil
			}
			free_node(it.node)
		}
		tree^.numValues -= 1
		return
	}

	assert(it.node^.numValues == minValues)
	parent, indexInParent := parent_node(it.node)

	left := parent^.children[indexInParent - 1] if indexInParent > 0 else nil
	right := parent^.children[indexInParent + 1] if indexInParent < parent^.numValues else nil
	assert(left != nil || right != nil)

	sibling := left if left != nil && (right == nil || left^.numValues >= right^.numValues) else right

	if sibling^.numValues > minValues {
		if sibling == left {
			// rotate an element from the left sibling
			copy(it.node^.values[1:it.valueIndex], it.node^.values[:it.valueIndex-1])
			it.node^.values[0] = parent^.values[indexInParent-1]
			if it.node^.children != nil {
				copy(it.node^.children[1:it.valueIndex+1], it.node^.children[0:it.valueIndex])
				set_child(it.node, 0, sibling^.children[sibling^.numValues])
			}
			parent^.values[indexInParent-1] = sibling^.values[sibling^.numValues-1]
		} else {
			// rotate an element from the right sibling
			copy(it.node^.values[it.valueIndex:it.node^.numValues-2], it.node^.values[it.valueIndex+1:it.node^.numValues-1])
			it.node^.values[it.node^.numValues-1] = parent^.values[indexInParent]
			if it.node^.children != nil {
				copy(it.node^.children[it.valueIndex+1:it.node^.numValues], it.node^.children[it.valueIndex+2:it.node^.numValues+1])
				set_child(it.node, it.node^.numValues, sibling^.children[0])
				copy(sibling^.children[0:sibling^.numValues], sibling^.children[1:sibling^.numValues+1])
			}
			parent^.values[indexInParent] = sibling^.values[0]
			copy(sibling^.values[0:sibling.numValues-1], sibling^.values[1:sibling.numValues])
		}
		sibling.numValues -= 1
		tree^.numValues -= 1
		return
	}

	// both sibling and node have the minimum number of elements, merge them
	itParent := Iterator{node = parent}
	if sibling == left {
		right = it.node
		itParent.valueIndex = indexInParent-1
	} else {
		assert(sibling == right)
		left = it.node
		itParent.valueIndex = indexInParent
	}
	remove_element(it)
	assert(left^.numValues + right^.numValues + 1 == Order - 1)
	left^.values[left.numValues] = parent^.values[itParent.valueIndex]
	left^.numValues += 1
	copy(left^.values[left.numValues:], right^.values[:right^.numValues])
	if left^.children != nil {
		set_children(left, left.numValues+1, right^.children[:right^.numValues+1])
	}
	free_node(right)

	// tail recursion
	erase(tree, itParent)
}