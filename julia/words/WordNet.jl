module WordNet

import Base: print, show
export load_db, get_pointed, get_synsets

abstract PointerDestination

@enum PartOfSpeech adj adv noun verb
@enum AdjMarker none predicate prenomial postnomial

type Pointer
  dest::PointerDestination
  label::Symbol
end

type Lemma <: PointerDestination
  word::AbstractString
  lexId::Int
  adjMarker::AdjMarker
  synset::PointerDestination
  verbFrames::Vector{Int}
  pointers::Vector{Pointer}
  Lemma(word, lexId, adjMarker, synset) = new(word, lexId, adjMarker, synset, Vector{Int}(), Vector{Pointer}())
end

function Base.print(io::IO, lemma::Lemma)
  print(io, "$(lemma.word) $(lemma.lexId) pointers: $(length(lemma.pointers)) {$(lemma.synset)}")
end

Base.show(io::IO, lemma::Lemma) = print(io, "($lemma)::Lemma")

type SynSet <: PointerDestination
  words::Vector{Lemma}
  pointers::Vector{Pointer}
  offset::Int
  part::PartOfSpeech
  satelite::Bool
  gloss::AbstractString
  lexFile::Int
  SynSet() = new(Vector{Lemma}(), Vector{Pointer}())
end

function Base.print(io::IO, synset::SynSet)
  print(io, "($(synset.offset), $(synset.part)) [")
  for w in synset.words
    print(io, " $(w.word),")
  end
  print(io, "] '$(synset.gloss)', pointers: $(length(synset.pointers))")
end

Base.show(io::IO, synset::SynSet) = print(io, "{", synset, "}::SynSet")

const lemmas = Dict{AbstractString, Vector{SynSet}}()
const synsets = Dict{Tuple{Int, PartOfSpeech}, SynSet}()

const part_markers = Dict(
  "n" => (noun, false),
  "v" => (verb, false),
  "a" => (adj, false),
  "s" => (adj, true),
  "r" => (adv, false))

const pointer_types = Dict(
  noun => [("!",  :antonym),
           ("@",  :hypernym),
           ("@i", :instance_hypernym),
           ("~",  :hyponym),
           ("~i", :instance_hyponym),
           ("#m", :member_holonym),
           ("#s", :substance_holonym),
           ("#p", :part_holonym),
           ("%m", :member_meronym),
           ("%s", :substance_meronym),
           ("%p", :part_meronym),
           ("\\", :derived_from_adj),
           ("=",  :attribute),
           ("+",  :derivationally_related),
           (";c", :domain_TOPIC),
           ("-c", :member_of_domain_TOPIC),
           (";r", :domain_REGION),
           ("-r", :member_of_domain_REGION),
           (";u", :domain_USAGE),
           ("-u", :member_of_domain_USAGE)],
  verb => [("!",  :antonym),
           ("@",  :hypernym),
           ("~",  :hyponym),
           ("*",  :entailment),
           (">",  :cause),
           ("^",  :also_see),
           ("\$", :verb_group),
           ("+",  :derivationally_related),
           (";c", :domain_TOPIC),
           (";r", :domain_REGION),
           (";u", :domain_USAGE)],
  adj =>  [("!",  :antonym),
           ("&",  :similar_to),
           ("<",  :participle_of_verb),
           ("\\", :pertains_to_noun),
           ("+",  :derivationally_related),
           ("=",  :attribute),
           ("^",  :also_see),
           (";c", :domain_TOPIC),
           (";r", :domain_REGION),
           (";u", :domain_USAGE)],
  adv =>  [("!",  :antonym),
           ("\\", :derived_from_adj),
           ("+",  :derivationally_related),
           (";c", :domain_TOPIC),
           (";r", :domain_REGION),
           (";u", :domain_USAGE)])

const adj_markers = Dict("p"  => predicate,
                         "a"  => prenomial,
                         "ip" => postnomial)

immutable DBPointer
  offset::Int
  part::PartOfSpeech
  label::Symbol
  src::Int
  tgt::Int
end

type DBSynSet
  synset::SynSet
  pointers::Vector{DBPointer}
  DBSynSet() = new(SynSet(), Vector{DBPointer}())
end

function parse_pointer(ptrStr::AbstractString, part::PartOfSpeech)
  for (str, sym) in pointer_types[part]
    if startswith(ptrStr, str)
      return sym
    end
  end
  @assert false "$ptrStr not a valid pointer for $part"
end

function parse_synset(s::AbstractString, dbSynsets::Vector{DBSynSet})
  dbSet = DBSynSet()
  res = match(r"(.*?)\|\s*(.*?)\s*$", s)
  elems = split(res.captures[1])
  dbSet.synset.gloss = string(res.captures[2])
  dbSet.synset.offset = parse(Int, elems[1])
  dbSet.synset.lexFile = parse(Int, elems[2])
  dbSet.synset.part, dbSet.synset.satelite = part_markers[elems[3]]
  wordCount = parse(Int, elems[4], 16)
  for w = 0:wordCount-1
    word = replace(elems[5 + 2w], '_', ' ')
    lexId = parse(Int, elems[5 + 2w + 1], 16)
    res = match(r"^(.*)\((.*)\)", word)
    adjMarker = none
    if res != nothing
      word = res.captures[1]
      adjMarker = adj_markers[res.captures[2]]
    end
    lemma = Lemma(word, lexId, adjMarker, dbSet.synset)
    push!(dbSet.synset.words, lemma)
    global lemmas
    wordSynsets = get!(()->Vector{SynSet}(), lemmas, word)
    push!(wordSynsets, dbSet.synset)
  end
  ptrCount = parse(Int, elems[5 + 2wordCount])
  ptrBase = 5 + 2wordCount + 1
  for p = 0:ptrCount-1
    label = parse_pointer(elems[ptrBase + 4p], dbSet.synset.part)
    offset = parse(Int, elems[ptrBase + 4p + 1])
    part, satelite = part_markers[elems[ptrBase + 4p + 2]]
    srctgt = elems[ptrBase + 4p + 3]
    @assert length(srctgt) == 4
    dbPtr = DBPointer(offset, part, label, parse(Int, srctgt[1:2], 16), parse(Int, srctgt[3:4], 16))
    push!(dbSet.pointers, dbPtr)
  end
  if dbSet.synset.part == verb
    frmCount = parse(Int, elems[ptrBase + 4ptrCount])
    frmBase = ptrBase + 4ptrCount + 1
    for f = 0:frmCount-1
      @assert elems[frmBase + 3f] == "+"
      frameNum = parse(Int, elems[frmBase + 3f + 1])
      wordInd = parse(Int, elems[frmBase + 3f + 2], 16)
      if wordInd == 0
        for w in dbSet.synset.words
          push!(w.verbFrames, frameNum)
        end
      else
        push!(dbSet.synset.words[wordInd].verbFrames, frameNum)
      end
    end
    @assert frmBase + 3frmCount - 1 == length(elems)
  else
    @assert ptrBase + 4ptrCount - 1 == length(elems)
  end
  global synsets
  push!(dbSynsets, dbSet)
  synsets[(dbSet.synset.offset, dbSet.synset.part)] = dbSet.synset
end

function load_file(filename::AbstractString, dbSynsets::Vector{DBSynSet})
  open(filename) do ios
    while !eof(ios)
      l = readline(ios)
      startswith(l, "  ") && continue
      parse_synset(l, dbSynsets)
    end
  end
end

function resolve_pointers(dbSynsets::Vector{DBSynSet})
  @assert !isempty(dbSynsets)
  for dbSet in dbSynsets
    for p in dbSet.pointers
      dstSet = synsets[(p.offset, p.part)]
      src = p.src == 0 ? dbSet.synset : dbSet.synset.words[p.src]
      dst = p.tgt == 0 ? dstSet : dstSet.words[p.tgt]
      push!(src.pointers, Pointer(dst, p.label))
    end
  end
end

function load_db(path::AbstractString = "wndb31")
  global lemmas, synsets
  empty!(lemmas)
  empty!(synsets)
  dbSynsets = Vector{DBSynSet}()
  for file in ["data.adj", "data.adv", "data.noun", "data.verb"]
    load_file(joinpath(path, file), dbSynsets)
  end
  resolve_pointers(dbSynsets)
end

function get_pointed(src::PointerDestination, label::Symbol; transitive = false)
  if !transitive
    return map(p->p.dest, filter(p->p.label == label, src.pointers))
  end
  visited = Set{PointerDestination}()
  tovisit = [src]
  result = Vector{PointerDestination}()
  while !isempty(tovisit)
    pointed = [map(s->get_pointed(s, label), tovisit)...]
    empty!(tovisit)
    for p in pointed
      if !in(p, visited)
        push!(visited, p)
        push!(tovisit, p)
        push!(result, p)
      end
    end
  end
  return result
end

function get_synsets(word::AbstractString, part = nothing)
  global lemmas
  sets = get(()->Vector{SynSet}(), lemmas, word)
  if isa(part, PartOfSpeech)
    sets = filter(s->s.part == part, sets)
  end
  return sets
end

end # module
