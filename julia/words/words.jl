module WordNetImp

import Base: ==, hash

@enum PartOfSpeech adj adv noun verb
@enum AdjMarker none predicate prenomial postnomial

enum_names{E <: Enum}(::Type{E}) = map(e->Symbol(string(e)), instances(E))

abstract PointerDestination

type Pointer
  dest::PointerDestination
  kind::Symbol
end

type Lemma <: PointerDestination
  file::AbstractString
  word::AbstractString
  lex_id::Int
  adj_marker::AdjMarker
  pointers::Vector{Pointer}
  synset::PointerDestination
  Lemma(file, word, lex_id) = new(file, word, lex_id, none, Vector{Pointer}())
end

get_part(filename::AbstractString) = first(filter(p->startswith(filename, string(p)), instances(PartOfSpeech)))
get_part(l::Lemma) = get_part(l.file)

hash(l::Lemma, h::UInt) = hash(l.file, hash(l.word, hash(l.lex_id, h)))
==(l1::Lemma, l2::Lemma) = l1.word == l2.word && l1.lex_id == l2.lex_id && l1.file == l2.file

global lemmas = Dict{Lemma, Lemma}()

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

type SynSet <: PointerDestination
  lemmas::Vector{Lemma}
  gloss::AbstractString
  part::PartOfSpeech
  frames::Vector{Int}
  pointers::Vector{Pointer}
end

const print_diag = true

function parse_word(words::AbstractString, filename::AbstractString)
  print_diag && info("parse_word: $words")
  res = match(r"^\s*([^,]*)(.*)", words)
  word = res.captures[1]
  words = res.captures[2]
  res = match(r"^(.*)\((.*?)\)\s*$", word)
  marker = none
  if res != nothing
    word = res.captures[1]
    marker = adj_markers[res.captures[2]]
  end
  res = match(r"^(.*):(.*)", word)
  if res != nothing
    filename = res.captures[1]
    word = res.captures[2]
  end
  res = match(r"\d+$", word)
  lexId = 0
  if res != nothing
    lexId = parse(res.match)
    word = SubString(word, 1, length(word) - length(res.match))
  end
  replace(word, ['_','"'], s->s=="_" ? " " : "")
  lemma = Lemma(filename, word, lexId)
  global lemmas
  lemma = get!(lemmas, lemma, lemma)
  if marker != none
    lemma.adj_marker = marker
  end
  return lemma, words
end

function parse_pointer(words::AbstractString, part::PartOfSpeech)
  print_diag && info("parse_pointer: $words")
  res = match(r"^\s*,\s*(.*)", words)
  ptrType = nothing
  if res != nothing
    words = res.captures[1]
    for (str, sym) in pointer_types[part]
      if startswith(words, str)
        ptrType = sym
        words = SubString(words, length(str) + 1)
        break
      end
    end
  end
  return ptrType, words
end

function parse_lemma(word::AbstractString, filename::AbstractString, part::PartOfSpeech)
  print_diag && info("parse_lemma: $word")
  lemma, word = parse_word(word, filename)
  res = match(r"^\s*,\s*(.*)", word)
  if res != nothing
    word = res.captures[1]
  end
  while true
    word = strip(word)
    isempty(word) && break
    ptrLemma, word = parse_word(word, filename)
    ptrType, word = parse_pointer(word, #=get_part(ptrLemma)=#part)
    push!(lemma.pointers, Pointer(ptrLemma, ptrType))
  end
  return lemma
end

function parse_synset(s::AbstractString, filename::AbstractString, part::PartOfSpeech)
  print_diag && info("parse_synset: $s")
  res = match(r"^\s*{(.*?)\((.*)\)\s*}(.*)", s)
  res == nothing && return nothing, s
  words = res.captures[1]
  gloss = res.captures[2]
  unmatched = res.captures[3]
  synset = SynSet(Vector{Lemma}(), gloss, part, Vector{Int}(), Vector{Pointer}())
  res = match(r"^(.*)frames.*:(.*)", words)
  if res != nothing
    words = res.captures[1]
    synset.frames = map(parse, split(res.captures[2], ','))
  end
  while true
    res = match(r"^\s*\[(.+?)\]\s*(.*)", words)
    if res == nothing
      res = match(r"^\s*([^,]+)(.*)", words)
    end
    res == nothing && break
    word = res.captures[1]
    words = res.captures[2]
    lemma = parse_lemma(word, filename, part)
    ptrType, words = parse_pointer(words, part)
    if ptrType == nothing
      lemma.synset = synset
      push!(synset.lemmas, lemma)
    else
      ptr = Pointer(lemma, ptrType)
      push!(synset.pointers, ptr)
    end
  end
  return synset, unmatched
end

function parse_file(path::AbstractString, filename::AbstractString, part::PartOfSpeech)
  info(filename)
  open(joinpath(path, filename)) do ios
    while !eof(ios)
      l = chomp(readline(ios))
      res = match(r"^\s*\[(.*)", l)
      if res != nothing
        l = res.captures[1]
        head = nothing
        while true
          synset, l = parse_synset(l, filename, part)
          if synset != nothing
            if head == nothing
              head = synset
            end
            if synset != head
              push!(synset.pointers, Pointer(head, :head_synset))
              push!(head.pointers, Pointer(synset, :satellite_synset))
            end
          end
          if match(r"^\s*-", l) != nothing
            head = nothing
          end
          if eof(ios) || match(r"^\s*\]", l) != nothing
            break
          end
          l = chomp(readline(ios))
        end
      else
        parse_synset(l, filename, part)
      end
    end
  end
end

function parse_db(path::AbstractString, filename = "")
  path = joinpath(path, "dbfiles")
  if !isempty(filename)
    parse_file(path, filename, get_part(filename))
    return
  end
  files = readdir(path)
  for part in instances(PartOfSpeech)
    partname = string(part, '.')
    all(filter(f->startswith(f, partname), files)) do filename
      parse_file(path, filename, part)
      return true
    end
  end
end

end # module
