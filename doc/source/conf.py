# -*- coding: utf-8 -*-
#
# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Imports and Path setup ---------------------------------------------------

import sys, os, re
import datetime

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
# Root of the gempython package
sys.path.insert(1, os.path.abspath("{}".format(os.getenv("PYTHONSOURCE"))))
# Scripts directory
sys.path.insert(1, os.path.abspath("{}/gempython/scripts".format(os.getenv("PYTHONSOURCE"))))

if os.getenv("USE_DOXYREST"):
    # path for doxyrest sphinx extensions
    sys.path.insert(
        1, "{:s}/share/doxyrest/sphinx".format(os.getenv("DOXYREST_PREFIX"))
    )

    import doxyrest

import sphinx_rtd_theme


# -- Project information -----------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
# needs_sphinx = '1.0'

# General information about the project.

project = u"cmsgemos"
author = u"CMS GEM DAQ project"
copyright = u"2017—{:d} {:s}".format(datetime.date.today().year, author)

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The full version, including alpha/beta/rc tags.
release = os.popen("git describe --abbrev=6 --dirty --always --tags").read().strip()
try:
    release = re.sub("^v", "", release,)  #'1.0.0'
    # The short X.Y version.
    version = "{0}.{1}".format(*release.split("."))  #'1.0'
except Exception as e:
    print(e)
    version = "untagged-{0}".format(release)
    pass  # release = "0.0.0"

print("Version {}".format(version))
print("Release {}".format(release))


# -- General configuration ----------------------------------------------------

# # Tell sphinx what the primary language being documented is.
# primary_domain = "cpp"

# # Tell sphinx what the pygments highlight language should be.
# highlight_language = "cpp"

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = ".rst"

# The encoding of source files.
# source_encoding = 'utf-8-sig'

# The master toctree document.
master_doc = "index"

# The reST default role (used for this markup: `text`) to use for all documents.
# default_role = None

# If true, sectionauthor and moduleauthor directives will be shown in the
# output. They are ignored by default.
# show_authors = False

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = "sphinx"


# -- Extension configuration --------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    ## sphinx extensions
    "sphinx.ext.coverage",
    "sphinx.ext.doctest",
    "sphinx.ext.extlinks",
    "sphinx.ext.imgmath",
    "sphinx.ext.intersphinx",
    "sphinx.ext.mathjax",
    "sphinx.ext.todo",
    "sphinx.ext.viewcode",
    ## sphinxcontrib
    "sphinxcontrib.napoleon",
    "sphinxcontrib.srclinks",
    ## sphinx external
    "sphinx_copybutton",
    "sphinx_rtd_theme",
    "sphinx_tabs.tabs",
    "m2r",
]

extensions += ["autoapi.extension"]
extensions += ["breathe", "exhale"]
if os.getenv("USE_DOXYREST"):
    extensions += ["doxyrest", "cpplexer"]

## -- autoapi python configuration --------------------------------------------

autoapi_type = "python"
autoapi_python_use_implicit_namespaces = True
autoapi_dirs = [
    "{}".format(os.getenv("PYTHONSOURCE"))
]
autoapi_add_toctree_entry = False
autoapi_keep_files = True
autoapi_options = [
    "members",
    "undoc-members",
    "private-members",
    "show-inheritance",
    "special-members",
    # "show-inheritance-diagram", ## currently fails in gempython
    "show-module-summary",
]
autoapi_ignore = ["*migrations*", "*conf.py", "*setup.py"]
autoapi_template_dir = "_templates/autoapi"

## -- Breathe+Exhale configuration --------------------------------------------

breathe_projects = {
    "cmsgemos": "../exhalebuild/xml",
}

breathe_default_project = "cmsgemos"

# Setup the exhale extension
exhale_args = {
    # These arguments are required
    "containmentFolder": "./exhale-api",
    "rootFileName": "api.rst",
    "rootFileTitle": "API documentation for cmsgemos",
    "doxygenStripFromPath": "{}".format(os.path.abspath("../../")),
    # Suggested optional arguments
    # "lexerMapping": {r".*\.md": "md",},
    "createTreeView": True,
    # "afterTitleDescription": "",
    # "fullApiSubSectionTitle": "",
    # "afterBodySummary": "",
    # "unabridgedOrphanKinds": [""],
    "fullToctreeMaxDepth": 1,
    # TIP: if using the sphinx-bootstrap-theme, you need
    "treeViewIsBootstrap": False,
    "exhaleExecutesDoxygen": True,
    "exhaleDoxygenStdin": """
PROJECT_NAME = cmsgemos
PROJECT_NUMBER = {}
REPEAT_BRIEF = YES
INHERIT_DOCS = YES
MARKDOWN_SUPPORT = YES
AUTOLINK_SUPPORT = YES
SUBGROUPING = YES
EXTRACT_LOCAL_CLASSES = YES
CASE_SENSE_NAMES = YES
SHOW_INCLUDE_FILES = YES
GENERATE_HTML = YES
INLINE_INFO = YES
SORT_MEMBER_DOCS = YES
GENERATE_DEPRECATEDLIST= YES
SHOW_USED_FILES = YES
SHOW_FILES = YES
SHOW_NAMESPACES = YES
USE_MDFILE_AS_MAINPAGE = ../../README.md
INPUT = ../../README.md \
        ../../gembase/include \
        ../../gemutils/include \
        ../../gemhardware/include \
        ../../gemreadout/include \
        ../../gemsupervisor/include \
        ../../gemonlinedb/include \
        ../../gempython/include \
        ../../gemcalibration/include
PREDEFINED+= DOXYGEN_IGNORE_THIS
""".format(
        release
    ),
}


## -- other extension configuration -------------------------------------------

# Disable numpy docstrings for Napoleon, because they eat headers such as
# "Examples"
napoleon_numpy_docstring = False

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = True


# -- Options for HTML output --------------------------------------------------

html_context = {
    "display_github": True,
    "github_host": "github.com",
    "github_user": "cms-gem-daq-project",
    "github_repo": "cmsgemos",
    "github_version": "release/legacy-1.0",
    "conf_py_path": "/doc/source/",
    "last_updated": "{}".format(os.popen("date -u +'%a %b %d %Y %T %Z'").read().strip()),
    "commit": "{}".format(os.popen("git describe --abbrev=8 --dirty --always").read().strip()),
}

html_show_sourcelink = True

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "sphinx_rtd_theme"

# The name for this set of Sphinx documents.  If None, it defaults to
# "<project> v<release> documentation".
html_title = "cmsgemos: Online Software for the CMS GEM Project"

# A shorter title for the navigation bar.  Default is the same as html_title.
html_short_title = "cmsgemos"

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
# html_logo = None

## Show only the logo at the top of the sidebar
# logo_only

# The name of an image file (within the static path) to use as favicon of the
# docs.  This file should be a Windows icon file (.ico) being 16x16 or 32x32
# pixels large.
# html_favicon = None

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
html_theme_options = {
    "navigation_depth": 50,
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]
# html_extra_path = ["../build/html"]

# Custom CSS
# These paths are either relative to html_static_path
# or fully qualified paths (eg. https://...)
html_css_files = [
    "{}/css/rtd-custom.css".format(os.getenv("GEM_DOCS_URL")),
]

# Custom JavaScript
# These paths are either relative to html_static_path
# or fully qualified paths (eg. https://...)
html_js_files = [
    # # enable for arrow key navigation
    # "{}/scripts/js/guides-navigation.js".format(os.getenv("GEM_DOCS_URL")),
]

# If not '', a 'Last updated on:' timestamp is inserted at every page bottom,
# using the given strftime format.
# html_last_updated_fmt = '%b %d, %Y'

# If true, the index is split into individual pages for each letter.
# html_split_index = False


# -- Options for HTMLHelp output ----------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = "cmsgemos-doc"


# -- Options for LaTeX output -------------------------------------------------

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    #'papersize': 'letterpaper',

    # The font size ('10pt', '11pt' or '12pt').
    #'pointsize': '10pt',

    # Additional stuff for the LaTeX preamble.
    #
    # 'preamble': '',
    # Latex figure (float) alignment
    #
    # 'figure_align': 'htbp',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (
        master_doc,
        "cmsgemos.tex",
        u"cmsgemos Documentation",
        u"cms-gem-online-sw@cern.ch",
        "manual",
    ),
]

# The name of an image file (relative to this directory) to place at the top of
# the title page.
# latex_logo = None

# For "manual" documents, if this is true, then toplevel headings are parts,
# not chapters.
# latex_use_parts = False

# If true, show page references after internal links.
# latex_show_pagerefs = False

# If true, show URL addresses after external links.
# latex_show_urls = False

# Documents to append as an appendix to all manuals.
# latex_appendices = []

# If false, no module index is generated.
# latex_domain_indices = True


# -- Options for manual page output -------------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [(master_doc, "cmsgemos", u"cmsgemos Documentation", [author], 1)]


# -- Options for Texinfo output -----------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (
        master_doc,
        "cmsgemos",
        u"cmsgemos Documentation",
        author,
        "cmsgemos",
        "One line description of project.",
        "Miscellaneous",
    ),
]

# Documents to append as an appendix to all manuals.
# texinfo_appendices = []

# -- Options for Epub output --------------------------------------------------

# Bibliographic Dublin Core info.
epub_title = project
epub_author = author
epub_publisher = author
epub_copyright = copyright

# The unique identifier of the text. This can be a ISBN number
# or the project homepage.
#
# epub_identifier = ''

# A unique identification for the text.
#
# epub_uid = ''

# A list of files that should not be packed into the epub file.
epub_exclude_files = ["search.html"]


# This config value contains the locations and names of other projects that
# should be linked to in this documentation.
intersphinx_mapping = {
    "gemplotting": (os.getenv("EOS_SITE_URL") + "/docs/api/gemplotting/latest", None,),
    "vfatqc": (os.getenv("EOS_SITE_URL") + "/docs/api/vfatqc/latest", None,),
    "ctp7_modules": (
        os.getenv("EOS_SITE_URL") + "/docs/api/ctp7_modules/latest",
        None,
    ),
    "reg_utils": (os.getenv("EOS_SITE_URL") + "/docs/api/reg_utils/latest", None,),
    "xhal": (os.getenv("EOS_SITE_URL") + "/docs/api/xhal/latest", None,),
}
