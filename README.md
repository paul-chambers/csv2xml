# csv2xml
Simple tool to convert a CSV file into the key="value" style required for XML.

Lines that start with punctuation are converted to XML comments

Blank lines are emitted as a single newline. A line consisting of only
empty fields is considered to be blank, also.

The first line that doesn't start with punctuation is assumed to be a
comma-separated list of the column labels, and is not emitted.

Subsequent lines are considered to be comma-separated values, which
are the values to be emitted. Each value is output in the form:

{column header} = "{value}"
