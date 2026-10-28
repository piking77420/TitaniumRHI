#ifndef TITANIUM_SPECIAL_MEMBERS_H
#define TITANIUM_SPECIAL_MEMBERS_H

// Use only when memberwise moves preserve the class's ownership invariants.
#define RHI_MOVE_ONLY(Type)                         \
    Type(const Type&) = delete;                    \
    Type& operator=(const Type&) = delete;         \
    Type(Type&&) noexcept = default;               \
    Type& operator=(Type&&) noexcept = default

#endif // TITANIUM_SPECIAL_MEMBERS_H
