/*
 * QueryFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_QUERY_FLAGS_H
#define LLGL_QUERY_FLAGS_H


#include <cstddef>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

//! Query type enumeration.
enum class QueryType
{
    SamplesPassed,                  //!< Number of samples that passed the depth test. This can be used as render condition.
    AnySamplesPassed,               //!< Non-zero if any samples passed the depth test. This can be used as render condition.
    AnySamplesPassedConservative,   //!< Non-zero if any samples passed the depth test within a conservative rasterization. This can be used as render condition.

    TimeElapsed,                    //!< Elapsed time (in nanoseconds) between the begin- and end query command.
    
    StreamOutPrimitivesWritten,     //!< Number of vertices that have been written into a stream output (also called "Transform Feedback").
    StreamOutOverflow,              //!< Non-zero if any of the streaming output buffers (also called "Transform Feedback Buffers") has an overflow.
    
    /**
    \brief Pipeline statistics such as number of shader invocations, generated primitives, etc.
    \see QueryPipelineStatistics
    */
    PipelineStatistics,
};


/* ----- Structures ----- */

/**
\brief Query data structure for pipeline statistics.
\remarks If the renderer does not support individual members of this structure, they will be set to QueryPipelineStatistics::invalidNum.
\see QueryType::PipelineStatistics
\see CommandBuffer::Query
*/
struct QueryPipelineStatistics
{
    //! Invalid number for unsupported members. This is the default value of all members.
    static const std::uint64_t  invalidNum  = static_cast<std::uint64_t>(-1);

    /**
    \brief Number of members in this structure: 12.
    \remarks This determines the number of individual query objects for OpenGL.
    */
    static const std::size_t    memberCount = 12;

    std::uint64_t numPrimitivesGenerated                = invalidNum; //!< Number of generated primitives which are send to the rasterizer (either emitted from the geometry or vertex shader).
    std::uint64_t numVerticesSubmitted                  = invalidNum; //!< Number of vertices submitted to the input-assembly.
    std::uint64_t numPrimitivesSubmitted                = invalidNum; //!< Number of primitives submitted to the input-assembly.
    std::uint64_t numVertexShaderInvocations            = invalidNum; //!< Number of vertex shader invocations.
    std::uint64_t numTessControlShaderInvocations       = invalidNum; //!< Number of tessellation-control shader invocations.
    std::uint64_t numTessEvaluationShaderInvocations    = invalidNum; //!< Number of tessellation-evaluation shader invocations.
    std::uint64_t numGeometryShaderInvocations          = invalidNum; //!< Number of geometry shader invocations.
    std::uint64_t numFragmentShaderInvocations          = invalidNum; //!< Number of fragment shader invocations.
    std::uint64_t numComputeShaderInvocations           = invalidNum; //!< Number of compute shader invocations.
    std::uint64_t numGeometryPrimitivesGenerated        = invalidNum; //!< Number of primitives generated by the geometry shader.
    std::uint64_t numClippingInputPrimitives            = invalidNum; //!< Number of primitives that reached the primitive clipping stage.
    std::uint64_t numClippingOutputPrimitives           = invalidNum; //!< Number of primitives that passed the primitive clipping stage.
};

//! Query descriptor structure.
struct QueryDescriptor
{
    QueryDescriptor() = default;

    QueryDescriptor(QueryType type, bool renderCondition = false) :
        type            { type            },
        renderCondition { renderCondition }
    {
    }

    //! Specifies the type of the query. By default QueryType::SamplesPassed (occlusion query).
    QueryType   type            = QueryType::SamplesPassed;

    /**
    \brief Specifies whether the query is to be used as a render condition. By default false.
    \remarks If this is true, 'type' can only have one of the following values:
    QueryType::SamplesPassed, QueryType::AnySamplesPassed, QueryType::AnySamplesPassedConservative, or QueryType::StreamOutOverflow.
    */
    bool        renderCondition = false;
};


} // /namespace LLGL


#endif



// ================================================================================
