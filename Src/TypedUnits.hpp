//
// Created by treys on 2023/03/12 20:50.
//

#ifndef TYPEDUNITS_HPP
#define TYPEDUNITS_HPP

//#define UNIT_ADD_CUSTOM_TYPE(namespaceName, nameSingular, namePlural, abbreviation, /*definition*/...)\
//	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _u8, namePlural ## _u8, abbreviation ## _u8, uint8_t, __VA_ARGS__) \
//        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _u16, namePlural ## _u16, abbreviation ## _u16, uint16_t, __VA_ARGS__) \
//        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _u32, namePlural ## _u32, abbreviation ## _u32, uint32_t, __VA_ARGS__) \
//        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _u64, namePlural ## _u64, abbreviation ## _u64, uint64_t, __VA_ARGS__) \
//        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _i8, namePlural ## _i8, abbreviation ## _i8, int8_t, __VA_ARGS__) \
//        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _i16, namePlural ## _i16, abbreviation ## _i16, int16_t, __VA_ARGS__) \
//        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _i32, namePlural ## _i32, abbreviation ## _i32, int32_t, __VA_ARGS__) \
//        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _i64, namePlural ## _i64, abbreviation ## _i64, int64_t, __VA_ARGS__) \
//        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _z, namePlural ## _z, abbreviation ## _z, size_t, __VA_ARGS__) \
//        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _f, namePlural ## _f, abbreviation ## _f, float, __VA_ARGS__) \
//        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _d, namePlural ## _d, abbreviation ## _d, double, __VA_ARGS__) \

#define UNIT_ADD_CUSTOM_TYPE(namespaceName, nameSingular, namePlural, abbreviation, /*definition*/...)\
        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _u32, namePlural ## _u32, abbreviation ## _u32, uint32_t, __VA_ARGS__) \
        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _i32, namePlural ## _i32, abbreviation ## _i32, int32_t, __VA_ARGS__) \
        UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular ## _f, namePlural ## _f, abbreviation ## _f, float, __VA_ARGS__)

#define UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular, namePlural, abbreviation, underlyingType, /*definition*/...)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nameSingular, namePlural, abbreviation, underlyingType, __VA_ARGS__)                             \
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, femto ## nameSingular, femto ## namePlural, f ## abbreviation, underlyingType, femto<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, pico ## nameSingular, pico ## namePlural, p ## abbreviation, underlyingType, pico<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, nano ## nameSingular, nano ## namePlural, n ## abbreviation, underlyingType, nano<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, micro ## nameSingular, micro ## namePlural, u ## abbreviation, underlyingType, micro<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, milli ## nameSingular, milli ## namePlural, m ## abbreviation, underlyingType, milli<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, centi ## nameSingular, centi ## namePlural, c ## abbreviation, underlyingType, centi<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, deci ## nameSingular, deci ## namePlural, d ## abbreviation, underlyingType, deci<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, deca ## nameSingular, deca ## namePlural, da ## abbreviation, underlyingType, deca<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, hecto ## nameSingular, hecto ## namePlural, h ## abbreviation, underlyingType, hecto<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, kilo ## nameSingular, kilo ## namePlural, k ## abbreviation, underlyingType, kilo<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, mega ## nameSingular, mega ## namePlural, M ## abbreviation, underlyingType, mega<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, giga ## nameSingular, giga ## namePlural, G ## abbreviation, underlyingType, giga<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, tera ## nameSingular, tera ## namePlural, T ## abbreviation, underlyingType, tera<namePlural>)\
	UNIT_ADD_WITH_CUSTOM_TYPE(namespaceName, peta ## nameSingular, peta ## namePlural, P ## abbreviation, underlyingType, peta<namePlural>)\

//#define UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(namespaceName, nameSingular, namePlural, abbreviation, /*definition*/...) \
//  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _u8, namePlural ## _u8, abbreviation ## _u8, uint8_t, __VA_ARGS__) \
//  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _u16, namePlural ## _u16, abbreviation ## _u16, uint16_t, __VA_ARGS__) \
//  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _u32, namePlural ## _u32, abbreviation ## _u32, uint32_t, __VA_ARGS__) \
//  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _u64, namePlural ## _u64, abbreviation ## _u64, uint64_t, __VA_ARGS__) \
//  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _i8, namePlural ## _i8, abbreviation ## _i8, int8_t, __VA_ARGS__) \
//  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _i16, namePlural ## _i16, abbreviation ## _i16, int16_t, __VA_ARGS__) \
//  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _i32, namePlural ## _i32, abbreviation ## _i32, int32_t, __VA_ARGS__) \
//  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _i64, namePlural ## _i64, abbreviation ## _i64, int64_t, __VA_ARGS__) \
//  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _z, namePlural ## _z, abbreviation ## _z, size_t, __VA_ARGS__) \
//  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _f, namePlural ## _f, abbreviation ## _f, float, __VA_ARGS__) \
//  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _d, namePlural ## _d, abbreviation ## _d, double, __VA_ARGS__) \

#define UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(namespaceName, nameSingular, namePlural, abbreviation, /*definition*/...) \
  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _u32, namePlural ## _u32, abbreviation ## _u32, uint32_t, __VA_ARGS__) \
  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _i32, namePlural ## _i32, abbreviation ## _i32, int32_t, __VA_ARGS__) \
  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(namespaceName, nameSingular ## _f, namePlural ## _f, abbreviation ## _f, float, __VA_ARGS__) \

namespace units {
//  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(frequency, hertz, hertz, Hz, unit<std::ratio<1>, units::category::frequency_unit>)

  //------------------------------
  //	LENGTH UNITS
  //------------------------------

  /**
	 * @namespace	units::length
	 * @brief		namespace for unit types and containers representing length values
	 * @details		The SI unit for length is `meters`, and the corresponding `base_unit` category is
	 *				`length_unit`.
	 * @anchor		lengthContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_LENGTH_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(length, meter, meters, m, unit<std::ratio<1>, units::category::length_unit>)
  UNIT_ADD_CUSTOM_TYPE(length, foot, feet, ft, unit<std::ratio<381, 1250>, meters>)
  UNIT_ADD_CUSTOM_TYPE(length, inch, inches, in, unit<std::ratio<1, 12>, feet>)
  UNIT_ADD_CUSTOM_TYPE(length, mil, mils, mil, unit<std::ratio<1000>, inches>)
  UNIT_ADD_CUSTOM_TYPE(length, mile,   miles,    mi,    unit<std::ratio<5280>, feet>)
  UNIT_ADD_CUSTOM_TYPE(length, nauticalMile, nauticalMiles, nmi, unit<std::ratio<1852>, meters>)
  UNIT_ADD_CUSTOM_TYPE(length, astronicalUnit, astronicalUnits, au, unit<std::ratio<149597870700>, meters>)
  UNIT_ADD_CUSTOM_TYPE(length, lightyear, lightyears, ly, unit<std::ratio<9460730472580800>, meters>)
  UNIT_ADD_CUSTOM_TYPE(length, parsec, parsecs, pc, unit<std::ratio<648000>, astronicalUnits, std::ratio<-1>>)
  UNIT_ADD_CUSTOM_TYPE(length, angstrom, angstroms, angstrom, unit<std::ratio<1, 10>, nanometers>)
  UNIT_ADD_CUSTOM_TYPE(length, cubit, cubits, cbt, unit<std::ratio<18>, inches>)
  UNIT_ADD_CUSTOM_TYPE(length, fathom, fathoms, ftm, unit<std::ratio<6>, feet>)
  UNIT_ADD_CUSTOM_TYPE(length, chain, chains, ch, unit<std::ratio<66>, feet>)
  UNIT_ADD_CUSTOM_TYPE(length, furlong, furlongs, fur, unit<std::ratio<10>, chains>)
  UNIT_ADD_CUSTOM_TYPE(length, hand, hands, hand, unit<std::ratio<4>, inches>)
  UNIT_ADD_CUSTOM_TYPE(length, league, leagues, lea, unit<std::ratio<3>, miles>)
  UNIT_ADD_CUSTOM_TYPE(length, nauticalLeague, nauticalLeagues, nl, unit<std::ratio<3>, nauticalMiles>)
  UNIT_ADD_CUSTOM_TYPE(length, yard, yards, yd, unit<std::ratio<3>, feet>)
#endif

  //------------------------------
  //	MASS UNITS
  //------------------------------

  /**
	 * @namespace	units::mass
	 * @brief		namespace for unit types and containers representing mass values
	 * @details		The SI unit for mass is `kilograms`, and the corresponding `base_unit` category is
	 *				`mass_unit`.
	 * @anchor		massContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_MASS_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(mass, gram, grams, g, unit<std::ratio<1, 1000>, units::category::mass_unit>)
  UNIT_ADD_CUSTOM_TYPE(mass, metric_ton, metric_tons, t, unit<std::ratio<1000>, kilograms>)
  UNIT_ADD_CUSTOM_TYPE(mass, pound, pounds, lb, unit<std::ratio<45359237, 100000000>, kilograms>)
  UNIT_ADD_CUSTOM_TYPE(mass, long_ton, long_tons, ln_t, unit<std::ratio<2240>, pounds>)
  UNIT_ADD_CUSTOM_TYPE(mass, short_ton, short_tons, sh_t, unit<std::ratio<2000>, pounds>)
  UNIT_ADD_CUSTOM_TYPE(mass, stone, stone, st, unit<std::ratio<14>, pounds>)
  UNIT_ADD_CUSTOM_TYPE(mass, ounce, ounces, oz, unit<std::ratio<1, 16>, pounds>)
  UNIT_ADD_CUSTOM_TYPE(mass, carat, carats, ct, unit<std::ratio<200>, milligrams>)
  UNIT_ADD_CUSTOM_TYPE(mass, slug, slugs, slug, unit<std::ratio<145939029, 10000000>, kilograms>)
#endif

  //------------------------------
  //	TIME UNITS
  //------------------------------

  /**
	 * @namespace	units::time
	 * @brief		namespace for unit types and containers representing time values
	 * @details		The SI unit for time is `seconds`, and the corresponding `base_unit` category is
	 *				`time_unit`.
	 * @anchor		timeContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_TIME_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(time, second, seconds, s, unit<std::ratio<1>, units::category::time_unit>)
//  UNIT_ADD_CUSTOM_TYPE(time, minute, minutes, min, unit<std::ratio<60>, seconds>)
//  UNIT_ADD_CUSTOM_TYPE(time, hour, hours, hr, unit<std::ratio<60>, minutes>)
//  UNIT_ADD_CUSTOM_TYPE(time, day, days, d, unit<std::ratio<24>, hours>)
//  UNIT_ADD_CUSTOM_TYPE(time, week, weeks, wk, unit<std::ratio<7>, days>)
//  UNIT_ADD_CUSTOM_TYPE(time, year, years, yr, unit<std::ratio<365>, days>)
//  UNIT_ADD_CUSTOM_TYPE(time, julian_year, julian_years, a_j,	unit<std::ratio<31557600>, seconds>)
//  UNIT_ADD_CUSTOM_TYPE(time, gregorian_year, gregorian_years, a_g, unit<std::ratio<31556952>, seconds>)
#endif

  //------------------------------
  //	ANGLE UNITS
  //------------------------------

  /**
	 * @namespace	units::angle
	 * @brief		namespace for unit types and containers representing angle values
	 * @details		The SI unit for angle is `radians`, and the corresponding `base_unit` category is
	 *				`angle_unit`.
	 * @anchor		angleContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_ANGLE_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(angle, radian, radians, rad, unit<std::ratio<1>, units::category::angle_unit>)
  UNIT_ADD_CUSTOM_TYPE(angle, degree, degrees, deg, unit<std::ratio<1, 180>, radians, std::ratio<1>>)
  UNIT_ADD_CUSTOM_TYPE(angle, arcminute, arcminutes, arcmin, unit<std::ratio<1, 60>, degrees>)
  UNIT_ADD_CUSTOM_TYPE(angle, arcsecond, arcseconds, arcsec, unit<std::ratio<1, 60>, arcminutes>)
  UNIT_ADD_CUSTOM_TYPE(angle, milliarcsecond, milliarcseconds, mas, milli<arcseconds>)
  UNIT_ADD_CUSTOM_TYPE(angle, turn, turns, tr, unit<std::ratio<2>, radians, std::ratio<1>>)
  UNIT_ADD_CUSTOM_TYPE(angle, gradian, gradians, gon, unit<std::ratio<1, 400>, turns>)
#endif

  //------------------------------
  //	UNITS OF CURRENT
  //------------------------------
  /**
	 * @namespace	units::current
	 * @brief		namespace for unit types and containers representing current values
	 * @details		The SI unit for current is `amperes`, and the corresponding `base_unit` category is
	 *				`current_unit`.
	 * @anchor		currentContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_CURRENT_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(current, ampere, amperes, A, unit<std::ratio<1>, units::category::current_unit>)
#endif

  //------------------------------
  //	UNITS OF TEMPERATURE
  //------------------------------

  // NOTE: temperature units have special conversion overloads, since they
  // require translations and aren't a reversible transform.

  /**
	 * @namespace	units::temperature
	 * @brief		namespace for unit types and containers representing temperature values
	 * @details		The SI unit for temperature is `kelvin`, and the corresponding `base_unit` category is
	 *				`temperature_unit`.
	 * @anchor		temperatureContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_TEMPERATURE_UNITS)
  UNIT_ADD_CUSTOM_TYPE(temperature, kelvin, kelvin, K, unit<std::ratio<1>, units::category::temperature_unit>)
  UNIT_ADD_CUSTOM_TYPE(temperature, celsius, celsius, degC, unit<std::ratio<1>, kelvin, std::ratio<0>, std::ratio<27315, 100>>)
  UNIT_ADD_CUSTOM_TYPE(temperature, fahrenheit, fahrenheit, degF, unit<std::ratio<5, 9>, celsius, std::ratio<0>, std::ratio<-160, 9>>)
  UNIT_ADD_CUSTOM_TYPE(temperature, reaumur, reaumur, Re, unit<std::ratio<10, 8>, celsius>)
  UNIT_ADD_CUSTOM_TYPE(temperature, rankine, rankine, Ra, unit<std::ratio<5, 9>, kelvin>)
#endif

  //------------------------------
  //	UNITS OF AMOUNT OF SUBSTANCE
  //------------------------------

  /**
	 * @namespace	units::substance
	 * @brief		namespace for unit types and containers representing substance values
	 * @details		The SI unit for substance is `moles`, and the corresponding `base_unit` category is
	 *				`substance_unit`.
	 * @anchor		substanceContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_SUBSTANCE_UNITS)
  UNIT_ADD_CUSTOM_TYPE(substance, mole, moles, mol, unit<std::ratio<1>, units::category::substance_unit>)
#endif

  //------------------------------
  //	UNITS OF LUMINOUS INTENSITY
  //------------------------------

  /**
	 * @namespace	units::luminous_intensity
	 * @brief		namespace for unit types and containers representing luminous_intensity values
	 * @details		The SI unit for luminous_intensity is `candelas`, and the corresponding `base_unit` category is
	 *				`luminous_intensity_unit`.
	 * @anchor		luminousIntensityContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_LUMINOUS_INTENSITY_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(luminous_intensity, candela, candelas, cd, unit<std::ratio<1>, units::category::luminous_intensity_unit>)
#endif

  //------------------------------
  //	UNITS OF SOLID ANGLE
  //------------------------------

  /**
	 * @namespace	units::solid_angle
	 * @brief		namespace for unit types and containers representing solid_angle values
	 * @details		The SI unit for solid_angle is `steradians`, and the corresponding `base_unit` category is
	 *				`solid_angle_unit`.
	 * @anchor		solidAngleContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_SOLID_ANGLE_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(solid_angle, steradian, steradians, sr, unit<std::ratio<1>, units::category::solid_angle_unit>)
  UNIT_ADD_CUSTOM_TYPE(solid_angle, degree_squared, degrees_squared, sq_deg, squared<angle::degrees>)
  UNIT_ADD_CUSTOM_TYPE(solid_angle, spat, spats, sp, unit<std::ratio<4>, steradians, std::ratio<1>>)
#endif

  //------------------------------
  //	FREQUENCY UNITS
  //------------------------------

  /**
	 * @namespace	units::frequency
	 * @brief		namespace for unit types and containers representing frequency values
	 * @details		The SI unit for frequency is `hertz`, and the corresponding `base_unit` category is
	 *				`frequency_unit`.
	 * @anchor		frequencyContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_FREQUENCY_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(frequency, hertz, hertz, Hz, unit<std::ratio<1>, units::category::frequency_unit>)
#endif

  //------------------------------
  //	VELOCITY UNITS
  //------------------------------

  /**
	 * @namespace	units::velocity
	 * @brief		namespace for unit types and containers representing velocity values
	 * @details		The SI unit for velocity is `meters_per_second`, and the corresponding `base_unit` category is
	 *				`velocity_unit`.
	 * @anchor		velocityContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_VELOCITY_UNITS)
  UNIT_ADD_CUSTOM_TYPE(velocity, meters_per_second, meters_per_second, mps, unit<std::ratio<1>, units::category::velocity_unit>)
  UNIT_ADD_CUSTOM_TYPE(velocity, feet_per_second, feet_per_second, fps, compound_unit<length::feet, inverse<time::seconds>>)
  UNIT_ADD_CUSTOM_TYPE(velocity, miles_per_hour, miles_per_hour, mph, compound_unit<length::miles, inverse<time::hour>>)
  UNIT_ADD_CUSTOM_TYPE(velocity, kilometers_per_hour, kilometers_per_hour, kph, compound_unit<length::kilometers, inverse<time::hour>>)
  UNIT_ADD_CUSTOM_TYPE(velocity, knot, knots, kts, compound_unit<length::nauticalMiles, inverse<time::hour>>)
#endif

  //------------------------------
  //	ANGULAR VELOCITY UNITS
  //------------------------------

  /**
	 * @namespace	units::angular_velocity
	 * @brief		namespace for unit types and containers representing angular velocity values
	 * @details		The SI unit for angular velocity is `radians_per_second`, and the corresponding `base_unit` category is
	 *				`angular_velocity_unit`.
	 * @anchor		angularVelocityContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_ANGULAR_VELOCITY_UNITS)
  UNIT_ADD_CUSTOM_TYPE(angular_velocity, radians_per_second, radians_per_second, rad_per_s, unit<std::ratio<1>, units::category::angular_velocity_unit>)
  UNIT_ADD_CUSTOM_TYPE(angular_velocity, degrees_per_second, degrees_per_second, deg_per_s, compound_unit<angle::degrees, inverse<time::seconds>>)
  UNIT_ADD_CUSTOM_TYPE(angular_velocity, revolutions_per_minute, revolutions_per_minute, rpm, unit<std::ratio<2, 60>, radians_per_second, std::ratio<1>>)
  UNIT_ADD_CUSTOM_TYPE(angular_velocity, milliarcseconds_per_year, milliarcseconds_per_year, mas_per_yr, compound_unit<angle::milliarcseconds, inverse<time::year>>)
#endif

  //------------------------------
  //	UNITS OF ACCELERATION
  //------------------------------

  /**
	 * @namespace	units::acceleration
	 * @brief		namespace for unit types and containers representing acceleration values
	 * @details		The SI unit for acceleration is `meters_per_second_squared`, and the corresponding `base_unit` category is
	 *				`acceleration_unit`.
	 * @anchor		accelerationContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_ACCELERATION_UNITS)
  UNIT_ADD_CUSTOM_TYPE(acceleration, meters_per_second_squared, meters_per_second_squared, mps_sq, unit<std::ratio<1>, units::category::acceleration_unit>)
  UNIT_ADD_CUSTOM_TYPE(acceleration, feet_per_second_squared, feet_per_second_squared, fps_sq, compound_unit<length::feet, inverse<squared<time::seconds>>>)
  UNIT_ADD_CUSTOM_TYPE(acceleration, standard_gravity, standard_gravity, SG, unit<std::ratio<980665, 100000>, meters_per_second_squared>)
#endif

  //------------------------------
  //	UNITS OF FORCE
  //------------------------------

  /**
	 * @namespace	units::force
	 * @brief		namespace for unit types and containers representing force values
	 * @details		The SI unit for force is `newtons`, and the corresponding `base_unit` category is
	 *				`force_unit`.
	 * @anchor		forceContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_FORCE_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(force, newton, newtons, N, unit<std::ratio<1>, units::category::force_unit>)
  UNIT_ADD_CUSTOM_TYPE(force, pound, pounds, lbf, compound_unit<mass::slug, length::foot, inverse<squared<time::seconds>>>)
  UNIT_ADD_CUSTOM_TYPE(force, dyne, dynes, dyn, unit<std::ratio<1, 100000>, newtons>)
  UNIT_ADD_CUSTOM_TYPE(force, kilopond, kiloponds, kp, compound_unit<acceleration::standard_gravity, mass::kilograms>)
  UNIT_ADD_CUSTOM_TYPE(force, poundal, poundals, pdl, compound_unit<mass::pound, length::foot, inverse<squared<time::seconds>>>)
#endif

  //------------------------------
  //	UNITS OF PRESSURE
  //------------------------------

  /**
	 * @namespace	units::pressure
	 * @brief		namespace for unit types and containers representing pressure values
	 * @details		The SI unit for pressure is `pascals`, and the corresponding `base_unit` category is
	 *				`pressure_unit`.
	 * @anchor		pressureContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_PRESSURE_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(pressure, pascal, pascals, Pa, unit<std::ratio<1>, units::category::pressure_unit>)
  UNIT_ADD_CUSTOM_TYPE(pressure, bar, bars, bar, unit<std::ratio<100>, kilo<pascals>>)
  UNIT_ADD_CUSTOM_TYPE(pressure, mbar, mbars, mbar, unit<std::ratio<1>, milli<bar>>)
  UNIT_ADD_CUSTOM_TYPE(pressure, atmosphere, atmospheres, atm, unit<std::ratio<101325>, pascals>)
  UNIT_ADD_CUSTOM_TYPE(pressure, pounds_per_square_inch, pounds_per_square_inch, psi, compound_unit<force::pounds, inverse<squared<length::inch>>>)
  UNIT_ADD_CUSTOM_TYPE(pressure, torr, torrs, torr, unit<std::ratio<1, 760>, atmospheres>)
#endif

  //------------------------------
  //	UNITS OF CHARGE
  //------------------------------

  /**
	 * @namespace	units::charge
	 * @brief		namespace for unit types and containers representing charge values
	 * @details		The SI unit for charge is `coulombs`, and the corresponding `base_unit` category is
	 *				`charge_unit`.
	 * @anchor		chargeContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_CHARGE_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(charge, coulomb, coulombs, C, unit<std::ratio<1>, units::category::charge_unit>)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(charge, ampere_hour, ampere_hours, Ah, compound_unit<current::ampere, time::hours>)
#endif

  //------------------------------
  //	UNITS OF ENERGY
  //------------------------------

  /**
	 * @namespace	units::energy
	 * @brief		namespace for unit types and containers representing energy values
	 * @details		The SI unit for energy is `joules`, and the corresponding `base_unit` category is
	 *				`energy_unit`.
	 * @anchor		energyContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_ENERGY_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(energy, joule, joules, J, unit<std::ratio<1>, units::category::energy_unit>)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(energy, calorie, calories, cal, unit<std::ratio<4184, 1000>, joules>)
  UNIT_ADD_CUSTOM_TYPE(energy, kilowatt_hour, kilowatt_hours, kWh, unit<std::ratio<36, 10>, megajoules>)
  UNIT_ADD_CUSTOM_TYPE(energy, watt_hour, watt_hours, Wh, unit<std::ratio<1, 1000>, kilowatt_hours>)
  UNIT_ADD_CUSTOM_TYPE(energy, british_thermal_unit, british_thermal_units, BTU, unit<std::ratio<105505585262, 100000000>, joules>)
  UNIT_ADD_CUSTOM_TYPE(energy, british_thermal_unit_iso, british_thermal_units_iso, BTU_iso, unit<std::ratio<1055056, 1000>, joules>)
  UNIT_ADD_CUSTOM_TYPE(energy, british_thermal_unit_59, british_thermal_units_59, BTU59, unit<std::ratio<1054804, 1000>, joules>)
  UNIT_ADD_CUSTOM_TYPE(energy, therm, therms, thm, unit<std::ratio<100000>, british_thermal_units_59>)
  UNIT_ADD_CUSTOM_TYPE(energy, foot_pound, foot_pounds, ftlbf, unit<std::ratio<13558179483314004, 10000000000000000>, joules>)
#endif

  //------------------------------
  //	UNITS OF POWER
  //------------------------------

  /**
	 * @namespace	units::power
	 * @brief		namespace for unit types and containers representing power values
	 * @details		The SI unit for power is `watts`, and the corresponding `base_unit` category is
	 *				`power_unit`.
	 * @anchor		powerContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_POWER_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(power, watt, watts, W, unit<std::ratio<1>, units::category::power_unit>)
  UNIT_ADD_CUSTOM_TYPE(power, horsepower, horsepower, hp, unit<std::ratio<7457, 10>, watts>)
//  UNIT_ADD_DECIBEL(power, watt, dBW) // TODO: Figure out these units
//  UNIT_ADD_DECIBEL(power, milliwatt, dBm)
#endif

  //------------------------------
  //	UNITS OF VOLTAGE
  //------------------------------

  /**
	 * @namespace	units::voltage
	 * @brief		namespace for unit types and containers representing voltage values
	 * @details		The SI unit for voltage is `volts`, and the corresponding `base_unit` category is
	 *				`voltage_unit`.
	 * @anchor		voltageContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_VOLTAGE_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(voltage, volt, volts, V, unit<std::ratio<1>, units::category::voltage_unit>)
  UNIT_ADD_CUSTOM_TYPE(voltage, statvolt, statvolts, statV, unit<std::ratio<1000000, 299792458>, volts>)
  UNIT_ADD_CUSTOM_TYPE(voltage, abvolt, abvolts, abV, unit<std::ratio<1, 100000000>, volts>)
#endif

  //------------------------------
  //	UNITS OF CAPACITANCE
  //------------------------------

  /**
	 * @namespace	units::capacitance
	 * @brief		namespace for unit types and containers representing capacitance values
	 * @details		The SI unit for capacitance is `farads`, and the corresponding `base_unit` category is
	 *				`capacitance_unit`.
	 * @anchor		capacitanceContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_CAPACITANCE_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(capacitance, farad, farads, F, unit<std::ratio<1>, units::category::capacitance_unit>)
#endif

  //------------------------------
  //	UNITS OF IMPEDANCE
  //------------------------------

  /**
	 * @namespace	units::impedance
	 * @brief		namespace for unit types and containers representing impedance values
	 * @details		The SI unit for impedance is `ohms`, and the corresponding `base_unit` category is
	 *				`impedance_unit`.
	 * @anchor		impedanceContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_IMPEDANCE_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(impedance, ohm, ohms, Ohm, unit<std::ratio<1>, units::category::impedance_unit>)
#endif

  //------------------------------
  //	UNITS OF CONDUCTANCE
  //------------------------------

  /**
	 * @namespace	units::conductance
	 * @brief		namespace for unit types and containers representing conductance values
	 * @details		The SI unit for conductance is `siemens`, and the corresponding `base_unit` category is
	 *				`conductance_unit`.
	 * @anchor		conductanceContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_CONDUCTANCE_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(conductance, siemens, siemens, S, unit<std::ratio<1>, units::category::conductance_unit>)
#endif

  //------------------------------
  //	UNITS OF MAGNETIC FLUX
  //------------------------------

  /**
	 * @namespace	units::magnetic_flux
	 * @brief		namespace for unit types and containers representing magnetic_flux values
	 * @details		The SI unit for magnetic_flux is `webers`, and the corresponding `base_unit` category is
	 *				`magnetic_flux_unit`.
	 * @anchor		magneticFluxContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_MAGNETIC_FLUX_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(magnetic_flux, weber, webers, Wb, unit<std::ratio<1>, units::category::magnetic_flux_unit>)
  UNIT_ADD_CUSTOM_TYPE(magnetic_flux, maxwell, maxwells, Mx, unit<std::ratio<1, 100000000>, webers>)
#endif

  //----------------------------------------
  //	UNITS OF MAGNETIC FIELD STRENGTH
  //----------------------------------------

  /**
	 * @namespace	units::magnetic_field_strength
	 * @brief		namespace for unit types and containers representing magnetic_field_strength values
	 * @details		The SI unit for magnetic_field_strength is `teslas`, and the corresponding `base_unit` category is
	 *				`magnetic_field_strength_unit`.
	 * @anchor		magneticFieldStrengthContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
  // Unfortunately `_T` is a WINAPI macro, so we have to use `_Te` as the tesla abbreviation.
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_MAGNETIC_FIELD_STRENGTH_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(magnetic_field_strength, tesla, teslas, Te, unit<std::ratio<1>, units::category::magnetic_field_strength_unit>)
  UNIT_ADD_CUSTOM_TYPE(magnetic_field_strength, gauss, gauss, G, compound_unit<magnetic_flux::maxwell, inverse<squared<length::centimeter>>>)
#endif

  //------------------------------
  //	UNITS OF INDUCTANCE
  //------------------------------

  /**
	 * @namespace	units::inductance
	 * @brief		namespace for unit types and containers representing inductance values
	 * @details		The SI unit for inductance is `henrys`, and the corresponding `base_unit` category is
	 *				`inductance_unit`.
	 * @anchor		inductanceContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_INDUCTANCE_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(inductance, henry, henries, H, unit<std::ratio<1>, units::category::inductance_unit>)
#endif

  //------------------------------
  //	UNITS OF LUMINOUS FLUX
  //------------------------------

  /**
	 * @namespace	units::luminous_flux
	 * @brief		namespace for unit types and containers representing luminous_flux values
	 * @details		The SI unit for luminous_flux is `lumens`, and the corresponding `base_unit` category is
	 *				`luminous_flux_unit`.
	 * @anchor		luminousFluxContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_LUMINOUS_FLUX_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(luminous_flux, lumen, lumens, lm, unit<std::ratio<1>, units::category::luminous_flux_unit>)
#endif

  //------------------------------
  //	UNITS OF ILLUMINANCE
  //------------------------------

  /**
	 * @namespace	units::illuminance
	 * @brief		namespace for unit types and containers representing illuminance values
	 * @details		The SI unit for illuminance is `luxes`, and the corresponding `base_unit` category is
	 *				`illuminance_unit`.
	 * @anchor		illuminanceContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_ILLUMINANCE_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(illuminance, lux, luxes, lx, unit<std::ratio<1>, units::category::illuminance_unit>)
  UNIT_ADD_CUSTOM_TYPE(illuminance, footcandle, footcandles, fc, compound_unit<luminous_flux::lumen, inverse<squared<length::foot>>>)
  UNIT_ADD_CUSTOM_TYPE(illuminance, lumens_per_square_inch, lumens_per_square_inch, lm_per_in_sq, compound_unit<luminous_flux::lumen, inverse<squared<length::inch>>>)
  UNIT_ADD_CUSTOM_TYPE(illuminance, phot, phots, ph, compound_unit<luminous_flux::lumens, inverse<squared<length::centimeter>>>)
#endif

  //------------------------------
  //	UNITS OF RADIATION
  //------------------------------

  /**
	 * @namespace	units::radiation
	 * @brief		namespace for unit types and containers representing radiation values
	 * @details		The SI units for radiation are:
	 *				- source activity:	becquerel
	 *				- absorbed dose:	gray
	 *				- equivalent dose:	sievert
	 * @anchor		radiationContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_RADIATION_UNITS)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(radiation, becquerel, becquerels, Bq, unit<std::ratio<1>, units::frequency::hertz>)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(radiation, gray, grays, Gy, compound_unit<energy::joules, inverse<mass::kilogram>>)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(radiation, sievert, sieverts, Sv, unit<std::ratio<1>, grays>)
  UNIT_ADD_CUSTOM_TYPE(radiation, curie, curies, Ci, unit<std::ratio<37>, gigabecquerels>)
  UNIT_ADD_CUSTOM_TYPE(radiation, rutherford, rutherfords, rd, unit<std::ratio<1>, megabecquerels>)
  UNIT_ADD_CUSTOM_TYPE(radiation, rad, rads, rads, unit<std::ratio<1>, centigrays>)
#endif

  //------------------------------
  //	UNITS OF TORQUE
  //------------------------------

  /**
	 * @namespace	units::torque
	 * @brief		namespace for unit types and containers representing torque values
	 * @details		The SI unit for torque is `newton_meters`, and the corresponding `base_unit` category is
	 *				`torque_units`.
	 * @anchor		torqueContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_TORQUE_UNITS)
  UNIT_ADD_CUSTOM_TYPE(torque, newton_meter, newton_meters, Nm, unit<std::ratio<1>, units::energy::joule>)
  UNIT_ADD_CUSTOM_TYPE(torque, foot_pound, foot_pounds, ftlb, compound_unit<length::foot, force::pounds>)
  UNIT_ADD_CUSTOM_TYPE(torque, foot_poundal, foot_poundals, ftpdl, compound_unit<length::foot, force::poundal>)
  UNIT_ADD_CUSTOM_TYPE(torque, inch_pound, inch_pounds, inlb, compound_unit<length::inch, force::pounds>)
  UNIT_ADD_CUSTOM_TYPE(torque, meter_kilogram, meter_kilograms, mkgf, compound_unit<length::meter, force::kiloponds>)
#endif

  //------------------------------
  //	AREA UNITS
  //------------------------------

  /**
	 * @namespace	units::area
	 * @brief		namespace for unit types and containers representing area values
	 * @details		The SI unit for area is `square_meters`, and the corresponding `base_unit` category is
	 *				`area_unit`.
	 * @anchor		areaContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_AREA_UNITS)
  UNIT_ADD_CUSTOM_TYPE(area, square_meter, square_meters, sq_m, unit<std::ratio<1>, units::category::area_unit>)
  UNIT_ADD_CUSTOM_TYPE(area, square_foot, square_feet, sq_ft, squared<length::feet>)
  UNIT_ADD_CUSTOM_TYPE(area, square_inch, square_inches, sq_in, squared<length::inch>)
  UNIT_ADD_CUSTOM_TYPE(area, square_mile, square_miles, sq_mi, squared<length::miles>)
  UNIT_ADD_CUSTOM_TYPE(area, square_kilometer, square_kilometers, sq_km, squared<length::kilometers>)
  UNIT_ADD_CUSTOM_TYPE(area, hectare, hectares, ha, unit<std::ratio<10000>, square_meters>)
  UNIT_ADD_CUSTOM_TYPE(area, acre, acres, acre, unit<std::ratio<43560>, square_feet>)
#endif

  //------------------------------
  //	UNITS OF VOLUME
  //------------------------------

  /**
	 * @namespace	units::volume
	 * @brief		namespace for unit types and containers representing volume values
	 * @details		The SI unit for volume is `cubic_meters`, and the corresponding `base_unit` category is
	 *				`volume_unit`.
	 * @anchor		volumeContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_VOLUME_UNITS)
  UNIT_ADD_CUSTOM_TYPE(volume, cubic_meter, cubic_meters, cu_m, unit<std::ratio<1>, units::category::volume_unit>)
  UNIT_ADD_CUSTOM_TYPE(volume, cubic_millimeter, cubic_millimeters, cu_mm, cubed<length::millimeter>)
  UNIT_ADD_CUSTOM_TYPE(volume, cubic_kilometer, cubic_kilometers, cu_km, cubed<length::kilometer>)
  UNIT_ADD_WITH_METRIC_PREFIXES_STANDARD_TYPES(volume, liter, liters, L, cubed<deci<length::meter>>)
  UNIT_ADD_CUSTOM_TYPE(volume, cubic_inch, cubic_inches, cu_in, cubed<length::inches>)
  UNIT_ADD_CUSTOM_TYPE(volume, cubic_foot, cubic_feet, cu_ft, cubed<length::feet>)
  UNIT_ADD_CUSTOM_TYPE(volume, cubic_yard, cubic_yards, cu_yd, cubed<length::yards>)
  UNIT_ADD_CUSTOM_TYPE(volume, cubic_mile, cubic_miles, cu_mi, cubed<length::miles>)
  UNIT_ADD_CUSTOM_TYPE(volume, gallon, gallons, gal, unit<std::ratio<231>, cubic_inches>)
  UNIT_ADD_CUSTOM_TYPE(volume, quart, quarts, qt, unit<std::ratio<1, 4>, gallons>)
  UNIT_ADD_CUSTOM_TYPE(volume, pint, pints, pt, unit<std::ratio<1, 2>, quarts>)
  UNIT_ADD_CUSTOM_TYPE(volume, cup, cups, c, unit<std::ratio<1, 2>, pints>)
  UNIT_ADD_CUSTOM_TYPE(volume, fluid_ounce, fluid_ounces, fl_oz, unit<std::ratio<1, 8>, cups>)
  UNIT_ADD_CUSTOM_TYPE(volume, barrel, barrels, bl, unit<std::ratio<42>, gallons>)
  UNIT_ADD_CUSTOM_TYPE(volume, bushel, bushels, bu, unit<std::ratio<215042, 100>, cubic_inches>)
  UNIT_ADD_CUSTOM_TYPE(volume, cord, cords, cord, unit<std::ratio<128>, cubic_feet>)
  UNIT_ADD_CUSTOM_TYPE(volume, cubic_fathom, cubic_fathoms, cu_fm, cubed<length::fathom>)
  UNIT_ADD_CUSTOM_TYPE(volume, tablespoon, tablespoons, tbsp, unit<std::ratio<1, 2>, fluid_ounces>)
  UNIT_ADD_CUSTOM_TYPE(volume, teaspoon, teaspoons, tsp, unit<std::ratio<1, 6>, fluid_ounces>)
  UNIT_ADD_CUSTOM_TYPE(volume, pinch, pinches, pinch, unit<std::ratio<1, 8>, teaspoons>)
  UNIT_ADD_CUSTOM_TYPE(volume, dash, dashes, dash, unit<std::ratio<1, 2>, pinches>)
  UNIT_ADD_CUSTOM_TYPE(volume, drop, drops, drop, unit<std::ratio<1, 360>, fluid_ounces>)
  UNIT_ADD_CUSTOM_TYPE(volume, fifth, fifths, fifth, unit<std::ratio<1, 5>, gallons>)
  UNIT_ADD_CUSTOM_TYPE(volume, dram, drams, dr, unit<std::ratio<1, 8>, fluid_ounces>)
  UNIT_ADD_CUSTOM_TYPE(volume, gill, gills, gi, unit<std::ratio<4>, fluid_ounces>)
  UNIT_ADD_CUSTOM_TYPE(volume, peck, pecks, pk, unit<std::ratio<1, 4>, bushels>)
  UNIT_ADD_CUSTOM_TYPE(volume, sack, sacks, sacks, unit<std::ratio<3>, bushels>)
  UNIT_ADD_CUSTOM_TYPE(volume, shot, shots, shots, unit<std::ratio<3, 2>, fluid_ounces>)
  UNIT_ADD_CUSTOM_TYPE(volume, strike, strikes, strikes, unit<std::ratio<2>, bushels>)
#endif

  //------------------------------
  //	UNITS OF DENSITY
  //------------------------------

  /**
	 * @namespace	units::density
	 * @brief		namespace for unit types and containers representing density values
	 * @details		The SI unit for density is `kilograms_per_cubic_meter`, and the corresponding `base_unit` category is
	 *				`density_unit`.
	 * @anchor		densityContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_DENSITY_UNITS)
  UNIT_ADD_CUSTOM_TYPE(density, kilograms_per_cubic_meter, kilograms_per_cubic_meter, kg_per_cu_m, unit<std::ratio<1>, units::category::density_unit>)
  UNIT_ADD_CUSTOM_TYPE(density, grams_per_milliliter, grams_per_milliliter, g_per_mL, compound_unit<mass::grams, inverse<volume::milliliter>>)
  UNIT_ADD_CUSTOM_TYPE(density, kilograms_per_liter, kilograms_per_liter, kg_per_L, unit<std::ratio<1>, compound_unit<mass::grams, inverse<volume::milliliter>>>)
  UNIT_ADD_CUSTOM_TYPE(density, ounces_per_cubic_foot, ounces_per_cubic_foot, oz_per_cu_ft, compound_unit<mass::ounces, inverse<volume::cubic_foot>>)
  UNIT_ADD_CUSTOM_TYPE(density, ounces_per_cubic_inch, ounces_per_cubic_inch, oz_per_cu_in, compound_unit<mass::ounces, inverse<volume::cubic_inch>>)
  UNIT_ADD_CUSTOM_TYPE(density, ounces_per_gallon, ounces_per_gallon, oz_per_gal, compound_unit<mass::ounces, inverse<volume::gallon>>)
  UNIT_ADD_CUSTOM_TYPE(density, pounds_per_cubic_foot, pounds_per_cubic_foot, lb_per_cu_ft, compound_unit<mass::pounds, inverse<volume::cubic_foot>>)
  UNIT_ADD_CUSTOM_TYPE(density, pounds_per_cubic_inch, pounds_per_cubic_inch, lb_per_cu_in, compound_unit<mass::pounds, inverse<volume::cubic_inch>>)
  UNIT_ADD_CUSTOM_TYPE(density, pounds_per_gallon, pounds_per_gallon, lb_per_gal, compound_unit<mass::pounds, inverse<volume::gallon>>)
  UNIT_ADD_CUSTOM_TYPE(density, slugs_per_cubic_foot, slugs_per_cubic_foot, slug_per_cu_ft, compound_unit<mass::slugs, inverse<volume::cubic_foot>>)
#endif

  //------------------------------
  //	UNITS OF CONCENTRATION
  //------------------------------

  /**
	 * @namespace	units::concentration
	 * @brief		namespace for unit types and containers representing concentration values
	 * @details		The SI unit for concentration is `parts_per_million`, and the corresponding `base_unit` category is
	 *				`scalar_unit`.
	 * @anchor		concentrationContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_CONCENTRATION_UNITS)
  UNIT_ADD_CUSTOM_TYPE(concentration, ppm, parts_per_million, ppm, unit<std::ratio<1, 1000000>, units::category::scalar_unit>)
  UNIT_ADD_CUSTOM_TYPE(concentration, ppb, parts_per_billion, ppb, unit<std::ratio<1, 1000>, parts_per_million>)
  UNIT_ADD_CUSTOM_TYPE(concentration, ppt, parts_per_trillion, ppt, unit<std::ratio<1, 1000>, parts_per_billion>)
  UNIT_ADD_CUSTOM_TYPE(concentration, percent, percent, pct, unit<std::ratio<1, 100>, units::category::scalar_unit>)
#endif

  //------------------------------
  //	UNITS OF DATA
  //------------------------------

  /**
	 * @namespace	units::data
	 * @brief		namespace for unit types and containers representing data values
	 * @details		The base unit for data is `bytes`, and the corresponding `base_unit` category is
	 *				`data_unit`.
	 * @anchor		dataContainers
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_DATA_UNITS)
  UNIT_ADD_WITH_METRIC_AND_BINARY_PREFIXES(data, byte, bytes, B, unit<std::ratio<1>, units::category::data_unit>)
  UNIT_ADD_CUSTOM_TYPE(data, exabyte, exabytes, EB, unit<std::ratio<1000>, petabytes>)
  UNIT_ADD_WITH_METRIC_AND_BINARY_PREFIXES(data, bit, bits, b, unit<std::ratio<1, 8>, byte>)
  UNIT_ADD_CUSTOM_TYPE(data, exabit, exabits, Eb, unit<std::ratio<1000>, petabits>)
#endif

  //------------------------------
  //	UNITS OF DATA TRANSFER
  //------------------------------

  /**
	* @namespace	units::data_transfer_rate
	* @brief		namespace for unit types and containers representing data values
	* @details		The base unit for data is `bytes`, and the corresponding `base_unit` category is
	*				`data_unit`.
	* @anchor		dataContainers
	* @sa			See unit_t for more information on unit type containers.
	*/
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_DATA_TRANSFER_RATE_UNITS)
  UNIT_ADD_WITH_METRIC_AND_BINARY_PREFIXES(data_transfer_rate, bytes_per_second, bytes_per_second, Bps, unit<std::ratio<1>, units::category::data_transfer_rate_unit>)
  UNIT_ADD_CUSTOM_TYPE(data_transfer_rate, exabytes_per_second, exabytes_per_second, EBps, unit<std::ratio<1000>, petabytes_per_second>)
  UNIT_ADD_WITH_METRIC_AND_BINARY_PREFIXES(data_transfer_rate, bits_per_second, bits_per_second, bps, unit<std::ratio<1, 8>, bytes_per_second>)
  UNIT_ADD_CUSTOM_TYPE(data_transfer_rate, exabits_per_second, exabits_per_second, Ebps, unit<std::ratio<1000>, petabits_per_second>)
#endif

  //------------------------------
  //	CONSTANTS
  //------------------------------

  /**
	 * @brief		namespace for physical constants like PI and Avogadro's Number.
	 * @sa			See unit_t for more information on unit type containers.
	 */
#if !defined(DISABLE_PREDEFINED_UNITS) || defined(ENABLE_PREDEFINED_CONSTANTS_UNITS)
  namespace constants
  {
    /**
		 * @name Unit Containers
		 * @anchor constantContainers
		 * @{
		 */
    using PI = unit<std::ratio<1>, dimensionless::scalar, std::ratio<1>>;

    static constexpr const unit_t<PI>																											pi(1);											///< Ratio of a circle's circumference to its diameter.
    static constexpr const velocity::meters_per_second_t																						c(299792458.0);									///< Speed of light in vacuum.
    static constexpr const unit_t<compound_unit<cubed<length::meters>, inverse<mass::kilogram>, inverse<squared<time::seconds>>>>				G(6.67408e-11);									///< Newtonian constant of gravitation.
    static constexpr const unit_t<compound_unit<energy::joule, time::seconds>>																	h(6.626070040e-34);								///< Planck constant.
    static constexpr const unit_t<compound_unit<force::newtons, inverse<squared<current::ampere>>>>												mu0(pi * 4.0e-7 * force::newton_t(1) / units::math::cpow<2>(current::ampere_t(1)));										///< vacuum permeability.
    static constexpr const unit_t<compound_unit<capacitance::farad, inverse<length::meter>>>													epsilon0(1.0 / (mu0 * math::cpow<2>(c)));		///< vacuum permitivity.
    static constexpr const impedance::ohm_t																										Z0(mu0 * c);									///< characteristic impedance of vacuum.
    static constexpr const unit_t<compound_unit<force::newtons, area::square_meter, inverse<squared<charge::coulomb>>>>							k_e(1.0 / (4 * pi * epsilon0));					///< Coulomb's constant.
    static constexpr const charge::coulomb_t																									e(1.6021766208e-19);							///< elementary charge.
    static constexpr const mass::kilogram_t																										m_e(9.10938356e-31);							///< electron mass.
    static constexpr const mass::kilogram_t																										m_p(1.672621898e-27);							///< proton mass.
    static constexpr const unit_t<compound_unit<energy::joules, inverse<magnetic_field_strength::tesla>>>										mu_B(e * h / (4 * pi *m_e));					///< Bohr magneton.
    static constexpr const unit_t<inverse<substance::mol>>																						N_A(6.022140857e23);							///< Avagadro's Number.
    static constexpr const unit_t<compound_unit<energy::joules, inverse<temperature::kelvin>, inverse<substance::moles>>>						R(8.3144598);									///< Gas constant.
    static constexpr const unit_t<compound_unit<energy::joules, inverse<temperature::kelvin>>>													k_B(R / N_A);									///< Boltzmann constant.
    static constexpr const unit_t<compound_unit<charge::coulomb, inverse<substance::mol>>>														F(N_A * e);										///< Faraday constant.
    static constexpr const unit_t<compound_unit<power::watts, inverse<area::square_meters>, inverse<squared<squared<temperature::kelvin>>>>>	sigma(5.6703671313131313e-8);	///< Stefan-Boltzmann constant.
                                                                                                                                                                                         /** @} */
  }
#endif

  //  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(frequency, hertz_u8, hertz_u8, Hz_u8, uint8_t, unit<std::ratio<1>, units::category::frequency_unit>)
  //  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(frequency, hertz_u16, hertz_u16, Hz_u16, uint16_t, unit<std::ratio<1>, units::category::frequency_unit>)
  //  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(frequency, hertz_u32, hertz_u32, Hz_u32, uint32_t, unit<std::ratio<1>, units::category::frequency_unit>)
  //  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(frequency, hertz_u64, hertz_u64, Hz_u64, uint64_t, unit<std::ratio<1>, units::category::frequency_unit>)
  //  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(frequency, hertz_i8, hertz_i8, Hz_i8, int8_t, unit<std::ratio<1>, units::category::frequency_unit>)
  //  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(frequency, hertz_i16, hertz_i16, Hz_i16, int16_t, unit<std::ratio<1>, units::category::frequency_unit>)
  //  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(frequency, hertz_i32, hertz_i32, Hz_i32, int32_t, unit<std::ratio<1>, units::category::frequency_unit>)
  //  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(frequency, hertz_i64, hertz_i64, Hz_i64, int64_t, unit<std::ratio<1>, units::category::frequency_unit>)
  //  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(frequency, hertz_z, hertz_z, Hz_z, size_t, unit<std::ratio<1>, units::category::frequency_unit>)
  //  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(frequency, hertz_f, hertz_f, Hz_f, float, unit<std::ratio<1>, units::category::frequency_unit>)
  //  UNIT_ADD_WITH_METRIC_PREFIXES_CUSTOM_TYPE(frequency, hertz_d, hertz_d, Hz_d, double, unit<std::ratio<1>, units::category::frequency_unit>)

  //  _u8, uint8_t
  //  _u16, uint16_t
  //  _u32, uint32_t
  //  _u64, uint64_t
  //  _i8, int8_t
  //  _i16, int16_t
  //  _i32, int32_t
  //  _i64, int64_t
  //  _z, size_t
}

//namespace units {
//  typedef unit_strong_t<unit<std::ratio<1>, base_unit<std::ratio<0, 1>, std::ratio<0>, std::ratio<-1>>>, uint32_t> hertz_u32_t;
//}


#endif  // TYPEDUNITS_HPP
