/*
Copyright (C) 2017-2020 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "transaction_item_reason.hpp"


namespace libdnf::transaction {


std::string TransactionItemReason_to_string(TransactionItemReason reason) {
    switch (reason) {
        case TransactionItemReason::UNKNOWN:
            return "unknown";
        case TransactionItemReason::DEPENDENCY:
            return "dependency";
        case TransactionItemReason::USER:
            return "user";
        case TransactionItemReason::CLEAN:
            return "clean";
        case TransactionItemReason::WEAK_DEPENDENCY:
            return "weak-dependency";
        case TransactionItemReason::GROUP:
            return "group";
    }
    return "";
}


static TransactionItemReason order[] = {
    TransactionItemReason::UNKNOWN,
    TransactionItemReason::CLEAN,
    TransactionItemReason::WEAK_DEPENDENCY,
    TransactionItemReason::DEPENDENCY,
    TransactionItemReason::GROUP,
    TransactionItemReason::USER,
};


bool operator<(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs == rhs) {
        return false;
    }
    for (auto i : order) {
        // iterate through 'order' and return according to which value matches
        if (lhs == i) {
            return true;
        }
        if (rhs == i) {
            return false;
        }
    }
    return false;
}


bool operator<=(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs == rhs) {
        return true;
    }
    return lhs < rhs;
}


bool operator>(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs == rhs) {
        return false;
    }
    return rhs < lhs;
}


bool operator>=(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs == rhs) {
        return true;
    }
    return lhs > rhs;
}


int TransactionItemReason_compare(TransactionItemReason lhs, TransactionItemReason rhs) {
    if (lhs < rhs) {
        return -1;
    } if (lhs > rhs) {
        return 1;
    } else {
        return 0;
    }
}


}  // namespace libdnf::transaction
