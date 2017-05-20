#ifndef __INTERVAL_TREE_H
#define __INTERVAL_TREE_H

#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;


template <class T, typename K = int>
class Interval {
public:
    K start;
    K stop;
    T value;
    Interval(K s, K e, const T& v)
        : start(s)
        , stop(e)
        , value(v)
    { }
};

template <class T, typename K>
K intervalStart(const Interval<T,K>& i) {
    return i.start;
}

template <class T, typename K>
K intervalStop(const Interval<T,K>& i) {
    return i.stop;
}

template <class T, typename K>
ostream& operator<<(ostream& out, Interval<T,K>& i) {
    out << "Interval(" << i.start << ", " << i.stop << "): " << i.value;
    return out;
}

template <class T, typename K = int>
class IntervalStartSorter {
public:
    bool operator() (const Interval<T,K>& a, const Interval<T,K>& b) {
        return a.start < b.start;
    }
};

template <class T, typename K = unsigned int>
class IntervalTree {

public:
    typedef Interval<T,K> interval;
    typedef vector<interval> intervalVector;
    typedef IntervalTree<T,K> intervalTree;
    
    intervalVector intervals;
    intervalTree* left;
    intervalTree* right;
    intervalTree* parent;
    K center; // median start value
    K maxstop; // maximum stop value (right extent)

    IntervalTree<T,K>(void)
        : left(NULL)
        , right(NULL)
        , parent(NULL)
        , center(0)
        , maxstop(0)
    { }

    IntervalTree<T,K>(const intervalTree& other) {
        center = other.center;
        maxstop = other.maxstop;
        intervals = other.intervals;
        if (other.left) {
            left = (intervalTree*) malloc(sizeof(intervalTree));
            *left = *other.left;
            left->parent = this;
        } else {
            left = NULL;
        }
        if (other.right) {
            right = new intervalTree();
            *right = *other.right;
            right->parent = this;
        } else {
            right = NULL;
        }
    }

    IntervalTree<T,K>& operator=(const intervalTree& other) {
        center = other.center;
        maxstop = other.maxstop;
        intervals = other.intervals;
        if (other.left) {
            left = new intervalTree();
            *left = *other.left;
            left->parent = this;
        } else {
            left = NULL;
        }
        if (other.right) {
            right = new intervalTree();
            *right = *other.right;
            right->parent = this;
        } else {
            right = NULL;
        }
        return *this;
    }

    IntervalTree<T,K>(
            intervalVector& ivals,
            unsigned int depth = 16,
            unsigned int minbucket = 64,
            K leftextent = 0,
            K rightextent = 0,
            unsigned int maxbucket = 512,
            intervalTree* p = NULL
            )
        : left(NULL)
        , right(NULL)
        , parent(p)
        , maxstop(0)
    {
        if (leftextent == 0 && rightextent == 0) {
            // sort intervals by start
            IntervalStartSorter<T,K> intervalStartSorter;
            sort(ivals.begin(), ivals.end(), intervalStartSorter);
        }

        --depth;
        if (depth == 0 || (ivals.size() < minbucket && ivals.size() < maxbucket)) {
            intervals = ivals;
            if(ivals.size() > 0){
              vector<K> stops;
              stops.resize(ivals.size());
              transform(ivals.begin(), ivals.end(), stops.begin(), intervalStop<T,K>);
              maxstop = *max_element(stops.begin(), stops.end());
            }
        } else {
            K leftp = 0;
            K rightp = 0;
            K centerp = 0;
            
            if (leftextent || rightextent) {
                leftp = leftextent;
                rightp = rightextent;
            } else {
                leftp = ivals.front().start;
                vector<K> stops;
                stops.resize(ivals.size());
                transform(ivals.begin(), ivals.end(), stops.begin(), intervalStop<T,K>);
                rightp = *max_element(stops.begin(), stops.end());
            }

            //centerp = ( leftp + rightp ) / 2;
            centerp = ivals.at(ivals.size() / 2).start;
            center = centerp;

            intervalVector lefts;
            intervalVector rights;

            for (typename intervalVector::iterator i = ivals.begin(); i != ivals.end(); ++i) {
                interval& interval = *i;
                if (interval.stop < center) {
                    lefts.push_back(interval);
                } else if (interval.start > center) {
                    rights.push_back(interval);
                } else {
                    intervals.push_back(interval);
                    if(interval.stop > maxstop)
                      maxstop = interval.stop;
                }
            }

            if (!lefts.empty()) {
                left = new intervalTree(lefts, depth, minbucket, leftp, centerp, maxbucket, this);
            }
            if (!rights.empty()) {
                right = new intervalTree(rights, depth, minbucket, centerp, rightp, maxbucket, this);
            }
        }
    }

    void findOverlapping(K start, K stop, intervalVector& overlapping) {
        if (!intervals.empty() && ! (stop < intervals.front().start)) {
            for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                interval& interval = *i;
                if (interval.stop >= start && interval.start <= stop) {
                    overlapping.push_back(interval);
                }
            }
        }

        if (left && start <= center) {
            left->findOverlapping(start, stop, overlapping);
        }

        if (right && stop >= center) {
            right->findOverlapping(start, stop, overlapping);
        }

    }

    typedef intervalTree* iterator;

    iterator begin(){
      return this;
    }

    iterator end(){
      return NULL;
    }

    iterator next(iterator i){
      if(i->left != NULL)
        return i->left;
      if(i->right != NULL)
        return i->right;

      while(1){
        if(i->parent == NULL)
          return end();
        if(i == i->parent->left && i->parent->right != NULL)
          return i->parent->right;
        i = i->parent;
      }
    }

    typedef pair<intervalTree*, typename intervalVector::iterator> OverlapIterator;

    OverlapIterator overlapIteratorBegin(K start, K stop){
      return overlapIteratorNext(start, stop, OverlapIterator(this, intervals.begin()), true);
    }

    OverlapIterator overlapIteratorEnd(){
      return OverlapIterator(NULL, intervals.end());
    }

    OverlapIterator overlapIteratorNext(K start, K stop, OverlapIterator oiter, bool checkCurrent=false){
      intervalTree* node = oiter.first;
      if (!node->intervals.empty() && ! (stop < node->intervals.front().start) && !(start > node->maxstop)) {
        if(!checkCurrent)
          oiter.second++;
        while(oiter.second != node->intervals.end()){
          interval& interval = *(oiter.second);
          if (interval.stop >= start && interval.start <= stop)
            return oiter;
          oiter.second++;
        }
      }

      /* Got to end of the interval vector, check children */
      if (node->left && start <= node->center) {
          return overlapIteratorNext(start, stop, OverlapIterator(node->left, node->left->intervals.begin()), true);
      }
      if (node->right && stop >= node->center) {
          return overlapIteratorNext(start, stop, OverlapIterator(node->right, node->right->intervals.begin()), true);
      }

      /* This is a leaf node, go back up the tree looking for unprocessed siblings */
      while(1){
        if(node->parent == NULL)
          return overlapIteratorEnd();
        if(node == node->parent->left && node->parent->right != NULL && stop >= node->parent->center)
          return overlapIteratorNext(start, stop, OverlapIterator(node->parent->right, node->parent->right->intervals.begin()), true);
        node = node->parent;
      }
    }

    void findOverlappingWithIterator(K start, K stop, intervalVector& overlapping){
      for(OverlapIterator i = overlapIteratorBegin(start, stop); i != overlapIteratorEnd(); i = overlapIteratorNext(start, stop, i)){
        overlapping.push_back(*i.second);
      }
    }

    void findContained(K start, K stop, intervalVector& contained) {
        if (!intervals.empty() && ! (stop < intervals.front().start)) {
            for (typename intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                interval& interval = *i;
                if (interval.start >= start && interval.stop <= stop) {
                    contained.push_back(interval);
                }
            }
        }

        if (left && start <= center) {
            left->findContained(start, stop, contained);
        }

        if (right && stop >= center) {
            right->findContained(start, stop, contained);
        }

    }

    void dumpTree(int depth){
      cout << "depth: " << depth << endl;
      for(auto i = intervals.begin(); i != intervals.end(); i++)
        cout << i->start << "," << i->stop << " ";
      cout << endl;
      if(left)
        left->dumpTree(depth+1);
      if(right)
        right->dumpTree(depth+1);
    }

    void dumpTreePointers(int depth=0){
      cout << "depth: " << depth << ", " << this << endl;
      if(left)
        left->dumpTreePointers(depth+1);
      if(right)
        right->dumpTreePointers(depth+1);
    }

    void printStats(int depth=0){
      cout << "depth: " << depth << ", " << intervals.size() << endl;
      if(left)
        left->printStats(depth+1);
      if(right)
        right->printStats(depth+1);
    }

    ~IntervalTree(void) {
        // traverse the left and right
        // delete them all the way down
        if (left) {
            delete left;
        }
        if (right) {
            delete right;
        }
    }

};

#endif
