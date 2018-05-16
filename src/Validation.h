#ifndef VALIDATION_H
#define VALIDATION_H

#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <functional>

namespace validation
{
	template <class T>
    struct Validator
	{
		virtual bool validate(const T& value) = 0;
        virtual std::string formatError() const = 0;
        virtual ~Validator() = default;
	};

	enum StringLengthOperation
	{
		Less,
		More,
		Equal
	};

	template <StringLengthOperation O>
	struct StringLengthValidator : public Validator<std::string>
	{
		size_t length;

		StringLengthValidator(size_t l) : length{ l } {}

		bool validate(const std::string& value) override { return validate(value, Int2Type<O>()); }
		std::string formatError() const override { return formatError(Int2Type<O>()); }

		template <int I>
		struct Int2Type
		{
			enum { value = I };
		};

		bool validate(const std::string& value, Int2Type<Less>) { return value.size() < length; }
		bool validate(const std::string& value, Int2Type<More>) { return value.size() > length; }
		bool validate(const std::string& value, Int2Type<Equal>) { return value.size() == length; }

		std::string formatError(Int2Type<Less>) const { return "Length should be less than " + std::to_string(length); }
		std::string formatError(Int2Type<More>) const { return "Length should be more than " + std::to_string(length); }
		std::string formatError(Int2Type<Equal>) const { return "Length should be equal to " + std::to_string(length); }
	};

	struct NonEmptyValidator : public Validator<std::string>
	{
		bool validate(const std::string& value) override { return !value.empty(); }
		std::string formatError() const override { return "Should not be empty"; }
	};

	struct NonZeroValidator : public Validator<int>
	{
		bool validate(const int& value) override { return value != 0; }
		std::string formatError() const override { return "Should not be zero"; }
	};

	template <class T>
	struct EqualToValidator : public Validator<T>
	{
		T equalTo;

		EqualToValidator(const T& equalTo) : equalTo{ equalTo } {}

		bool validate(const T& value) override { return value == equalTo; }
		std::string formatError() const override { return "Should be equal to " + std::to_string(equalTo); }
	};

	template <class T>
	struct CustomValidator : public Validator<T>
	{
		using ValidationFunc = std::function< bool(const T&) >;
		ValidationFunc validationFunc;

		using FormatErrorFunc = std::function< std::string() >;
		FormatErrorFunc formatErrorFunc;

		CustomValidator(ValidationFunc validationFunc, FormatErrorFunc formatErrorFunc) : validationFunc{ validationFunc }, formatErrorFunc{ formatErrorFunc } {}

		bool validate(const T& value) override
		{
			return validationFunc(value);
		}

		std::string formatError() const override
		{
			return formatErrorFunc();
		}
	};

	template <class T>
	struct ValidatorBuilder
	{
		using ValidatorPtr = std::unique_ptr<Validator<T>>;
		using Validators = std::vector< ValidatorPtr >;

		Validators& validators;

		ValidatorBuilder(Validators& validators) : validators{ validators } {}

		using Type = ValidatorBuilder<T>;

		Type& EqualTo(const T& value)
		{
			ValidatorPtr validator(new EqualToValidator<T>(value));
			validators.push_back(std::move(validator));

			return *this;
		}

		Type& NonZero()
		{
			ValidatorPtr validator(new NonZeroValidator());
			validators.push_back(std::move(validator));

			return *this;
		}

		template <StringLengthOperation O>
		Type& Length(size_t length)
		{
			ValidatorPtr validator(new StringLengthValidator<O>(length));
			validators.push_back(std::move(validator));

			return *this;
		}

		Type& NonEmpty()
		{
			ValidatorPtr validator(new NonEmptyValidator());
			validators.push_back(std::move(validator));

			return *this;
		}

		Type& Custom(typename CustomValidator<T>::ValidationFunc validationFunc, typename CustomValidator<T>::FormatErrorFunc formatErrorFunc)
		{
			ValidatorPtr validator(new CustomValidator<T>(validationFunc, formatErrorFunc));
			validators.push_back(std::move(validator));

			return *this;
		}
	};

	using FieldId = uint8_t;
	using ValidatorId = uint8_t;
	using ValidationResult = std::tuple<FieldId, ValidatorId>;
	constexpr ValidationResult NoError = { 0, 0 };

	struct Dummy {};

	template <class T, class D = Dummy>
	struct ValidationTraits
	{
	};

} // namespace validation

#define VALIDATE_BEGIN_(C, D)																									\
	namespace validation																										\
	{																															\
		template <>																												\
		struct ValidationTraits<C, D>																							\
		{																														\
			using Type = C;																										\
			struct FieldValidatorBase																							\
            {																													\
                std::string name;																								\
                virtual ValidatorId validate(const Type& t) = 0;																\
                virtual std::string getError(ValidatorId validatorId) const = 0;                                                \
                virtual ~FieldValidatorBase() = default;                                                                        \
            };																													\
			template <class T, T Type::*mem>																			\
			struct FieldValidator : public FieldValidatorBase																	\
			{																													\
				using Validators = typename ValidatorBuilder<T>::Validators;													\
				Validators validators;																							\
																																\
				ValidatorId validate(const Type& t) override																	\
				{																												\
					const T* member = &(t.*mem);																				\
																																\
					for (ValidatorId i = 0; i < validators.size(); ++i)															\
					{																											\
						auto& v = validators[i];																				\
						bool ok = v->validate(*member);																			\
						if ( !ok )																								\
							return i + 1;																						\
					}																											\
																																\
					return 0;																									\
				}																												\
				std::string getError(ValidatorId validatorId) const override { return validators[validatorId]->formatError(); }			\
			};																													\
																																\
			static ValidationResult validate(const Type& t)																		\
			{																													\
				static auto& fields = getFields();																						\
				for (FieldId i = 0; i < fields.size(); ++i)																		\
				{																												\
					auto f = fields[i].get();																					\
					ValidatorId validatorId = f->validate(t);																	\
					if ( validatorId != 0 )																						\
						return { i + 1, validatorId };																			\
				}																												\
				return NoError;																									\
			}																													\
																																\
			static std::tuple<std::string, std::string> formatErrorMessage(const ValidationResult& error)						\
			{																													\
				FieldId fieldId = std::get<0>(error) - 1;																		\
				ValidatorId validatorId = std::get<1>(error) - 1;																\
				return std::make_tuple(getFields()[fieldId]->name, getFields()[fieldId]->getError(validatorId));				\
			}																													\
																																\
			using FieldValidatorPtr = std::unique_ptr<FieldValidatorBase>;														\
			using Fields = std::vector< FieldValidatorPtr >;																	\
																																\
			static Fields& getFields()																							\
			{																													\
				static Fields fields;																							\
				{

#define VALIDATE_BEGIN(C)	VALIDATE_BEGIN_(C, Dummy)

#define VALIDATE_BEGIN_EX(C, D) struct D{}; VALIDATE_BEGIN_(C, D)

#define VALIDATE(F)																												\
				}																												\
				{																												\
					using MemberType = decltype(Type::F);																		\
					using ValidatorType = FieldValidator< MemberType, &Type::F >;												\
					ValidatorType* validator = new ValidatorType();																\
					validator->name = #F;																						\
					FieldValidatorPtr ptr(validator);																			\
					fields.push_back( std::move(ptr) );																			\
					ValidatorBuilder<MemberType> builder(validator->validators);												\
					builder

#define VALIDATE_END()																											\
				}																												\
				return fields;																									\
			}																													\
		};																														\
	}

#endif // VALIDATION_H
