#!/usr/bin/env bash

# This will generate a nice set of ctags for issuetracker
# Make sure that you have exuberant-ctags installed and not the
# lame GNU or BSD one
# add the following to your ~/.vimrc:
#    tags=~/.tags,~/.optimizer_tags,tags
#    path+=s,~/.optimizer_tags,tags
# Then to jup to an identifier put the cursor over it and type CTRL-]
# To jump to the next identifier type :tn
# You can also look up any tag by typing :tag foo
# You may want to add this directory to your path in vim:
# set path+=<issue-tracking repository>/**
# Happy tagging!

/usr/bin/env ctags \
  --fields=+iaS \
  --c++-kinds=+p \
  --extra=+q \
  --languages=c++ \
  --append=yes \
  --sort=yes \
  --recurse=yes \
  --exclude=.git \
  --exclude=*.class \
  -o ~/.optimizer_tags.tmp \
  $(pwd)

mv ~/.optimizer_tags.tmp ~/.optimizer_tags
