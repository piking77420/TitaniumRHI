#ifndef TITANIUM_SPECIAL_MEMBERS_H
#define TITANIUM_SPECIAL_MEMBERS_H

#define RHI_MOVE_ONLY(Type)                                                                                            \
    Type(const Type&) = delete;                                                                                        \
    Type& operator=(const Type&) = delete;                                                                             \
    Type(Type&&) noexcept = default;                                                                                   \
    Type& operator=(Type&&) noexcept = default

#endif // TITANIUM_SPECIAL_MEMBERS_H
