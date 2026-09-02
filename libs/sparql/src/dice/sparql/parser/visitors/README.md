## SPARQL Queries to Operand Dependency Graphs

For the construction of the operand dependency graph of a SPARQL query, the QueryUpdateVisitor follows a **recursive** 
approach. Ultimately, the generated operand dependency graph is a graph representation of the WHERE clause of the provided query and captures the operations
(e.g., join, left join and cartesian join) that need to be carried during query evaluation.

Currently, a recursive step stops in one of the following functions:

- `visitSubSelect` (It contains a nested visitor)
- `visitVerbSimple` (Endpoint for triple patterns)
- `visitPathElt` (Endpoint for triple patterns; currently does not consider path modifiers)
- `visitFilter` (Might contain a nested visitor for the _EXISTS_ function)
- `visitInlineDataOneVar`
- `visitInlineDataFull`

The functions listed above generate a single dependency graph, which in most cases consists of a single vertex (operand).
The generated dependency graphs are then combined and merged in a bottom-up fashion. The vertices of an operand dependency
graph store information that allows us to identify whether a vertex represents a triple patter, a filter, a subquery or 
inline data. The different types of vertices are not important for the construction.

#### Example (Single Basic Graph Pattern)
Consider the following graph pattern: 

`{ ?s ?p ?o . ?s <p1> <o1> . ?s <p2> <o2> . }`.

For the first triple pattern, `visitVerbSimple` will create a dependency graph _G1_. Similarly, `visitPathElt` will
create the graphs _G2_ and _G3_ for the second and third triple pattern, respectively. The resulting graphs will be then
iteratively combined (see function `combine_and_merge`).

`Gtemp = combine_and_merge(G1, G2); Gfinal = combine_and_merge(Gtemp, G3)`

--

`combine_and_merge` uses methods provided by the OperandDependencyGraph class and the **combine** part works as follows.
First, both graphs are copied to a new graph. Note here that the edges of the original graphs are persisted in the new graph.
After the graphs are copied, new dependencies are created between the operands of the graphs that are found in their respective
independent strong components (recall that an independent strong component contains the non-optional part of a query). 
In the example above, _Gfinal_ will contain bidirectional dependencies between all vertices; bidirectional edges denote
join operations (or filters). The **merge** part comes into play when a query contains UNION patterns.

#### Example (UNION graph pattern)
Consider the following graph pattern:

`{ { ?s <p1> <o1> } UNION { ?s <p2> <o2> } { ?s <p3> <o3> } UNION { ?s <p4> <o4> } }`.

For the first union pattern, the visitor will create a graph _G1_, which contains two vertices, as follows.
First, the graphs _G1a_ and _G1b_ are created that represent the two graph patterns of the union pattern. In turn, these
graphs are simply merged and **not** combined, as there are no dependencies between the two graphs (each group graph pattern
is evaluated independently of the other). Hence, _G1_ is **disconnected**. A **disconnected** graph _G2_ is created for the second union graph
pattern similarly. To construct the dependency graph of the original graph pattern, `combine_and_merge` distributes the union
operations over the join operation. More specifically, if combines each connected component of the first graph (_G1_) with every
connected component of the second graph (_G2_). This process generates four graphs which are then **merged**, thus resulting
in a single graph of four connected components. The equivalent pattern is shown below.

`{ ?s <p1> <o1> . ?s <p3> <o3> } UNION { ?s <p1> <o1> .?s <p4> <o4> } UNION { ?s <p2> <o2> . ?s <p3> <o3> } UNION { ?s <p2> <o2> . ?s <p4> <o4> }`.


--

In case of OPTIONAL patterns, `combine_and_merge` is replaced by `combine_optional`, which creates unidirectional dependencies.
Unidirectional edges denote left join operations. The difference between `combine_optional` and `combine_and_merge` is that 
the former does not distribute the union found in the optional part in the non-optional part. Before combining the 
non-optional part with the optional part, the visitor creates edges that capture the cartesian joins between different OPTIONAL patterns

#### Example (OPTIONAL graph pattern)
`{ ?s ?p ?o OPTIONAL { ?s <p> <o> } OPTIONAL { ?s <p1> <o1> }`

Here, the function `optional_cartesian_connections` is used first to combine the optional patterns and then the final graph is 
created by combining the non-optional part with the optional via the function _combine_optional_. 

#### Example (UNION within OPTIONAL)
`{ ?s ?p ?o OPTIONAL { { ?s <p> <o> } UNION { ?s <p1> <o1> } }`

The graph of the OPTIONAL part is disconnected. `combine_and_merge` would create a disconnected graph with four vertices.
`combine_optional` creates a connected graph with three vertices. The independent strong component of the resulting graph
contains only one vertex, which represents the non-optional part of the query (i.e., `?s ?p ?o`). Once, the non-optional part
is evaluated, the remaining graph will be disconnected, thus persisting the original UNION pattern.
