#pragma once

#include <atomic>

namespace VE
{
	class ReferenceCount
	{
	public:
		void IncreaseReferenceCount() const
		{
			m_ReferenceCount++;
		}
		void DecreaseReferenceCount() const
		{
			m_ReferenceCount--;
		}

		uint32_t GetReferenceCount() const
		{
			return m_ReferenceCount.load();
		}

	private:
		mutable std::atomic<uint32_t> m_ReferenceCount = 0;
	};

	namespace RefUtils
	{
		void AddToLiveReferences( void* instance );
		void RemoveFromLiveReferences( void* instance );
		bool IsLive( void* instance );
	}

	template<typename T>
	class Ref
	{
	public:
		Ref()
			: m_Instance( nullptr )
		{
		}

		Ref( std::nullptr_t n )
			: m_Instance( nullptr )
		{
		}

		Ref( T* instance )
			: m_Instance( instance )
		{
			static_assert( std::is_base_of<ReferenceCount, T>::value, "Class is not ReferenceCount!" );
			IncreaseRef();
		}

		template<typename T2>
		Ref( const Ref<T2>& other )
		{
			m_Instance = ( T* )other.m_Instance;
			IncreaseRef();
		}

		template<typename T2>
		Ref( Ref<T2>&& other )
		{
			m_Instance = ( T* )other.m_Instance;
			other.m_Instance = nullptr;
		}

		static Ref<T> CopyWithoutIncrement( const Ref<T>& other )
		{
			Ref<T> result = nullptr;
			result.m_Instance = other.m_Instance;
			return result;
		}

		~Ref()
		{
			DecreaseRef();
		}

		Ref( const Ref<T>& other )
			: m_Instance( other.m_Instance )
		{
			IncreaseRef();
		}

		Ref& operator=( std::nullptr_t )
		{
			DecreaseRef();
			m_Instance = nullptr;
			return *this;
		}

		Ref& operator=( const Ref<T>& other )
		{
			other.IncreaseRef();
			DecreaseRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=( const Ref<T2>& other )
		{
			other.IncreaseRef();
			DecreaseRef();

			m_Instance = other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=( Ref<T2>&& other )
		{
			DecreaseRef();

			m_Instance = other.m_Instance;
			other.m_Instance = nullptr;
			return *this;
		}

		operator bool()
		{
			return m_Instance != nullptr;
		}
		operator bool() const
		{
			return m_Instance != nullptr;
		}

		T* operator->()
		{
			return m_Instance;
		}
		const T* operator->() const
		{
			return m_Instance;
		}

		T& operator*()
		{
			return *m_Instance;
		}
		const T& operator*() const
		{
			return *m_Instance;
		}

		T* Raw()
		{
			return  m_Instance;
		}
		const T* Raw() const
		{
			return  m_Instance;
		}

		void Reset( T* instance = nullptr )
		{
			DecreaseRef();
			m_Instance = instance;
		}

		template<typename T2>
		Ref<T2> As() const
		{
			return Ref<T2>( *this );
		}

		template<typename... Args>
		static Ref<T> Create( Args&&... args )
		{
			return Ref<T>( new T( std::forward<Args>( args )... ) );
		}

		bool operator==( const Ref<T>& other ) const
		{
			return m_Instance == other.m_Instance;
		}

		bool operator!=( const Ref<T>& other ) const
		{
			return !( *this == other );
		}

		bool EqualsObject( const Ref<T>& other )
		{
			if ( !m_Instance || !other.m_Instance )
				return false;

			return *m_Instance == *other.m_Instance;
		}
	private:
		void IncreaseRef() const
		{
			if ( m_Instance )
			{
				m_Instance->IncreaseReferenceCount();
				RefUtils::AddToLiveReferences( ( void* )m_Instance );
			}
		}

		void DecreaseRef() const
		{
			if ( m_Instance )
			{
				m_Instance->DecreaseReferenceCount();
				if ( m_Instance->GetReferenceCount() == 0 )
				{
					delete m_Instance;
					RefUtils::RemoveFromLiveReferences( ( void* )m_Instance );
					m_Instance = nullptr;
				}
			}
		}

		template<class T2>
		friend class Ref;
		mutable T* m_Instance;
	};

	template<typename T>
	class WeakRef
	{
	public:
		WeakRef() = default;

		WeakRef( Ref<T> weakRef )
		{
			m_Instance = weakRef.Raw();
		}

		WeakRef( T* instance )
		{
			m_Instance = instance;
		}

		bool IsValid() const
		{
			return m_Instance ? RefUtils::IsLive( m_Instance ) : false;
		}
		operator bool() const
		{
			return IsValid();
		}
	private:
		T* m_Instance = nullptr;
	};
}
