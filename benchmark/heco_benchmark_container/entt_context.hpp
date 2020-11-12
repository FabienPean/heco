#define ENTT_STANDARD_CPP
#include<entt.hpp>

namespace entt {
    struct Context {
        struct variable_data {
            id_type type_id;
            std::unique_ptr<void, void(*)(void*)> value;
        };

        std::vector<variable_data> vars{};

        /**
        * @brief Binds an object to the context of the registry.
        *
        * If the value already exists it is overwritten, otherwise a new instance
        * of the given type is created and initialized with the arguments provided.
        *
        * @tparam Type Type of object to set.
        * @tparam Args Types of arguments to use to construct the object.
        * @param args Parameters to use to initialize the value.
        * @return A reference to the newly created object.
        */
        template<typename Type, typename... Args>
        Type& set(Args&&... args) {
            unset<Type>();
            vars.push_back(variable_data{ type_info<Type>::id(), { new Type{std::forward<Args>(args)...}, [](void* instance) { delete static_cast<Type*>(instance); } } });
            return *static_cast<Type*>(vars.back().value.get());
        }

        /**
         * @brief Unsets a context variable if it exists.
         * @tparam Type Type of object to set.
         */
        template<typename Type>
        void unset() {
            vars.erase(std::remove_if(vars.begin(), vars.end(), [](auto&& var) {
                return var.type_id == type_info<Type>::id();
                }), vars.end());
        }

        /**
         * @brief Binds an object to the context of the registry.
         *
         * In case the context doesn't contain the given object, the parameters
         * provided are used to construct it.
         *
         * @tparam Type Type of object to set.
         * @tparam Args Types of arguments to use to construct the object.
         * @param args Parameters to use to initialize the object.
         * @return A reference to the object in the context of the registry.
         */
        template<typename Type, typename... Args>
        Type& ctx_or_set(Args&&... args) {
            auto* value = try_ctx<Type>();
            return value ? *value : set<Type>(std::forward<Args>(args)...);
        }

        /**
         * @brief Returns a pointer to an object in the context of the registry.
         * @tparam Type Type of object to get.
         * @return A pointer to the object if it exists in the context of the
         * registry, a null pointer otherwise.
         */
        template<typename Type>
        const Type* try_ctx() const {
            auto it = std::find_if(vars.cbegin(), vars.cend(), [](auto&& var) { return var.type_id == type_info<Type>::id(); });
            return it == vars.cend() ? nullptr : static_cast<const Type*>(it->value.get());
        }

        /*! @copydoc try_ctx */
        template<typename Type>
        Type* try_ctx() {
            return const_cast<Type*>(std::as_const(*this).template try_ctx<Type>());
        }

        /**
         * @brief Returns a reference to an object in the context of the registry.
         *
         * @warning
         * Attempting to get a context variable that doesn't exist results in
         * undefined behavior.<br/>
         * An assertion will abort the execution at runtime in debug mode in case of
         * invalid requests.
         *
         * @tparam Type Type of object to get.
         * @return A valid reference to the object in the context of the registry.
         */
        template<typename Type>
        const Type& ctx() const {
            const auto* instance = try_ctx<Type>();
            ENTT_ASSERT(instance);
            return *instance;
        }

        /*! @copydoc ctx */
        template<typename Type>
        Type& ctx() {
            return const_cast<Type&>(std::as_const(*this).template ctx<Type>());
        }

        /**
         * @brief Visits a registry and returns the types for its context variables.
         *
         * The signature of the function should be equivalent to the following:
         *
         * @code{.cpp}
         * void(const id_type);
         * @endcode
         *
         * Returned identifiers are those of the context variables currently set.
         *
         * @sa type_info
         *
         * @warning
         * It's not specified whether a context variable created during the visit is
         * returned or not to the caller.
         *
         * @tparam Func Type of the function object to invoke.
         * @param func A valid function object.
         */
        template<typename Func>
        void ctx(Func func) const {
            for (auto pos = vars.size(); pos; --pos) {
                func(vars[pos - 1].type_id);
            }
        }
    };
}