/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     | Version:     4.0
    \\  /    A nd           | Web:         http://www.foam-extend.org
     \\/     M anipulation  | For copyright notice see file Copyright
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "interpolationTable.H"
#include "IFstream.H"

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

template<class Type>
void Foam::interpolationTable<Type>::readTable()
{
    // preserve the original (unexpanded) fileName to avoid absolute paths
    // appearing subsequently in the write() method
    fileName fName(fileName_);

    fName.expand();

    // Read data from file
    IFstream(fName)() >> *this;

    // Check that the data are okay
    check();

    if (this->empty())
    {
        FatalErrorIn
        (
            "Foam::interpolationTable<Type>::readTable()"
        )   << "table is empty" << nl
            << exit(FatalError);
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::interpolationTable<Type>::interpolationTable()
:
    List<Tuple2<scalar, Type> >(),
    boundsHandling_(interpolationTable::CLAMP),
    fileName_("fileNameIsUndefined")
{}


template<class Type>
Foam::interpolationTable<Type>::interpolationTable
(
    const List<Tuple2<scalar, Type> >& values,
    const boundsHandling bounds,
    const fileName& fName
)
:
    List<Tuple2<scalar, Type> >(values),
    boundsHandling_(bounds),
    fileName_(fName)
{}


template<class Type>
Foam::interpolationTable<Type>::interpolationTable(const fileName& fName)
:
    List<Tuple2<scalar, Type> >(),
    boundsHandling_(interpolationTable::CLAMP),
    fileName_(fName)
{
    readTable();
}


template<class Type>
Foam::interpolationTable<Type>::interpolationTable(const dictionary& dict)
:
    List<Tuple2<scalar, Type> >(),
    boundsHandling_(wordToBoundsHandling(dict.lookup("outOfBounds"))),
    fileName_(dict.lookup("fileName"))
{
    readTable();
}


template<class Type>
Foam::interpolationTable<Type>::interpolationTable
(
     const interpolationTable& interpTable
)
:
    List<Tuple2<scalar, Type> >(interpTable),
    boundsHandling_(interpTable.boundsHandling_),
    fileName_(interpTable.fileName_)
{}



// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
Foam::word Foam::interpolationTable<Type>::boundsHandlingToWord
(
     const boundsHandling& bound
) const
{
    word enumName("clamp");

    switch (bound)
    {
        case interpolationTable::EXIT:
        {
            enumName = "exit";
            break;
        }
        case interpolationTable::WARN:
        {
            enumName = "warn";
            break;
        }
        case interpolationTable::CLAMP:
        {
            enumName = "clamp";
            break;
        }
        case interpolationTable::REPEAT:
        {
            enumName = "repeat";
            break;
        }
    }

    return enumName;
}


template<class Type>
typename Foam::interpolationTable<Type>::boundsHandling
Foam::interpolationTable<Type>::wordToBoundsHandling
(
    const word& bound
) const
{
    if (bound == "exit")
    {
        return interpolationTable::EXIT;
    }
    else if (bound == "warn")
    {
        return interpolationTable::WARN;
    }
    else if (bound == "clamp")
    {
        return interpolationTable::CLAMP;
    }
    else if (bound == "repeat")
    {
        return interpolationTable::REPEAT;
    }
    else
    {
        WarningIn
        (
            "Foam::interpolationTable<Type>::wordToBoundsHandling(const word&)"
        )   << "bad outOfBounds specifier " << bound << " using 'warn'" << endl;

        return interpolationTable::CLAMP;
    }
}


template<class Type>
typename Foam::interpolationTable<Type>::boundsHandling
Foam::interpolationTable<Type>::outOfBounds
(
    const boundsHandling& bound
)
{
    boundsHandling prev = boundsHandling_;
    boundsHandling_ = bound;
    return prev;
}


template<class Type>
void Foam::interpolationTable<Type>::check() const
{
    label n = this->size();
    scalar prevValue = List<Tuple2<scalar, Type> >::operator[](0).first();

    for (label i=1; i<n; ++i)
    {
        const scalar currValue =
            List<Tuple2<scalar, Type> >::operator[](i).first();

        // avoid duplicate values (divide-by-zero error)
        if (currValue <= prevValue)
        {
            FatalErrorIn
            (
                "Foam::interpolationTable<Type>::checkOrder() const"
            )   << "out-of-order value: "
                << currValue << " at index " << i << nl
                << exit(FatalError);
        }
        prevValue = currValue;
    }
}


template<class Type>
void Foam::interpolationTable<Type>::write(Ostream& os) const
{
    os.writeKeyword("fileName")
        << fileName_ << token::END_STATEMENT << nl;
    os.writeKeyword("outOfBounds")
        << boundsHandlingToWord(boundsHandling_) << token::END_STATEMENT << nl;
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

template<class Type>
const Foam::Tuple2<Foam::scalar, Type>&
Foam::interpolationTable<Type>::operator[](const label i) const
{
    label ii = i;
    label n  = this->size();

    if (n <= 1)
    {
        ii = 0;
    }
    else if (ii < 0)
    {
        switch (boundsHandling_)
        {
            case interpolationTable::EXIT:
            {
                FatalErrorIn
                (
                    "Foam::interpolationTable<Type>::operator[]"
                    "(const label) const"
                )   << "index (" << ii << ") underflow" << nl
                    << exit(FatalError);
                break;
            }
            case interpolationTable::WARN:
            {
                WarningIn
                (
                    "Foam::interpolationTable<Type>::operator[]"
                    "(const label) const"
                )   << "index (" << ii << ") underflow" << nl
                    << "    Continuing with the first entry"
                    << endl;
                // fall-through to 'CLAMP'
            }
            case interpolationTable::CLAMP:
            {
                ii = 0;
                break;
            }
            case interpolationTable::REPEAT:
            {
                while (ii < 0)
                {
                    ii += n;
                }
                break;
            }
        }
    }
    else if (ii >= n)
    {
        switch (boundsHandling_)
        {
            case interpolationTable::EXIT:
            {
                FatalErrorIn
                (
                    "Foam::interpolationTable<Type>::operator[]"
                    "(const label) const"
                )   << "index (" << ii << ") overflow" << nl
                    << exit(FatalError);
                break;
            }
            case interpolationTable::WARN:
            {
                WarningIn
                (
                    "Foam::interpolationTable<Type>::operator[]"
                    "(const label) const"
                )   << "index (" << ii << ") overflow" << nl
                    << "    Continuing with the last entry"
                    << endl;
                // fall-through to 'CLAMP'
            }
            case interpolationTable::CLAMP:
            {
                ii = n - 1;
                break;
            }
            case interpolationTable::REPEAT:
            {
                while (ii >= n)
                {
                    ii -= n;
                }
                break;
            }
        }
    }

    return List<Tuple2<scalar, Type> >::operator[](ii);
}


template<class Type>
Type Foam::interpolationTable<Type>::operator()(const scalar value) const
{
    label n = this->size();

    if (n <= 1)
    {
        return List<Tuple2<scalar, Type> >::operator[](0).second();
    }

    scalar minLimit = List<Tuple2<scalar, Type> >::operator[](0).first();
    scalar maxLimit = List<Tuple2<scalar, Type> >::operator[](n-1).first();
    scalar lookupValue = value;

    if (lookupValue < minLimit)
    {
        switch (boundsHandling_)
        {
            case interpolationTable::EXIT:
            {
                FatalErrorIn
                (
                    "Foam::interpolationTable<Type>::operator[]"
                    "(const scalar) const"
                )   << "value (" << lookupValue << ") underflow" << nl
                    << exit(FatalError);
                break;
            }
            case interpolationTable::WARN:
            {
                WarningIn
                (
                    "Foam::interpolationTable<Type>::operator[]"
                    "(const scalar) const"
                )   << "value (" << lookupValue << ") underflow" << nl
                    << "    Continuing with the first entry"
                    << endl;
                // fall-through to 'CLAMP'
            }
            case interpolationTable::CLAMP:
            {
                return List<Tuple2<scalar, Type> >::operator[](0).second();
                break;
            }
            case interpolationTable::REPEAT:
            {
                // adjust lookupValue to >= 0
                while (lookupValue < 0)
                {
                    lookupValue += maxLimit;
                }
                break;
            }
        }
    }
    else if (lookupValue >= maxLimit)
    {
        switch (boundsHandling_)
        {
            case interpolationTable::EXIT:
            {
                FatalErrorIn
                (
                    "Foam::interpolationTable<Type>::operator[]"
                    "(const label) const"
                )   << "value (" << lookupValue << ") overflow" << nl
                    << exit(FatalError);
                break;
            }
            case interpolationTable::WARN:
            {
                WarningIn
                (
                    "Foam::interpolationTable<Type>::operator[]"
                    "(const label) const"
                )   << "value (" << lookupValue << ") overflow" << nl
                    << "    Continuing with the last entry"
                    << endl;
                // fall-through to 'CLAMP'
            }
            case interpolationTable::CLAMP:
            {
                return List<Tuple2<scalar, Type> >::operator[](n-1).second();
                break;
            }
            case interpolationTable::REPEAT:
            {
                // adjust lookupValue <= maxLimit
                while (lookupValue > maxLimit)
                {
                    lookupValue -= maxLimit;
                }
                break;
            }
        }
    }

    label lo = 0;
    label hi = 0;

    // look for the correct range
    for (label i = 0; i < n; ++i)
    {
        if (lookupValue >= List<Tuple2<scalar, Type> >::operator[](i).first())
        {
            lo = hi = i;
        }
        else
        {
            hi = i;
            break;
        }
    }

    if (lo == hi)
    {
        // we are at the end of the table - or there is only a single entry
        return List<Tuple2<scalar, Type> >::operator[](hi).second();
    }
    else if (hi == 0)
    {
        // this treatment should should only occur under these conditions:
        //  -> the 'REPEAT' treatment
        //  -> (0 <= value <= minLimit)
        //  -> minLimit > 0
        // Use the value at maxLimit as the value for value=0
        lo = n - 1;

        return
        (
            List<Tuple2<scalar, Type> >::operator[](lo).second()
          + (
                List<Tuple2<scalar, Type> >::operator[](hi).second()
              - List<Tuple2<scalar, Type> >::operator[](lo).second()
            )
           *(lookupValue / minLimit)
        );
    }
    else
    {
        // normal interpolation
        return
        (
            List<Tuple2<scalar, Type> >::operator[](lo).second()
          + (
                List<Tuple2<scalar, Type> >::operator[](hi).second()
              - List<Tuple2<scalar, Type> >::operator[](lo).second()
            )
           *(
                lookupValue
              - List<Tuple2<scalar, Type> >::operator[](lo).first()
            )
           /(
                List<Tuple2<scalar, Type> >::operator[](hi).first()
              - List<Tuple2<scalar, Type> >::operator[](lo).first()
            )
        );
    }
}


// ************************************************************************* //
