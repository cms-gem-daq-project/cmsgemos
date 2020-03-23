# -*- coding: utf-8 -*-
#
# cmsgemos documentation build configuration file, created by
# sphinx-quickstart on Wed Aug  2 20:53:00 2017.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import sys, os, re

sys.path.insert(1, os.path.abspath("../../gempython/pkg"))
sys.path.insert(1, os.path.abspath("../../gempython/pkg/gempython/scripts"))

if os.getenv("USE_DOXYREST"):
    # path for doxyrest sphinx extensions
    sys.path.insert(
        1, "{:s}/share/doxyrest/sphinx".format(os.getenv("DOXYREST_PREFIX"))
    )

    import doxyrest

import sphinx_rtd_theme

# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# General information about the project.
project = u"cmsgemos"
author = u"CMS GEM DAQ project"
copyright = u"2017, {}".format(author)

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The full version, including alpha/beta/rc tags.
release = os.popen("git describe --abbrev=6 --dirty --always --tags").read().strip()
try:
    release = re.sub("^v", "", release)  # '1.0.0'
except Exception as e:
    print(e)
    release = "0.0.0"

# The short X.Y version.
version = "{0}.{1}".format(*release.split("."))  # '1.0'
print("Version {}".format(version))
print("Release {}".format(release))

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.doctest",
    "sphinx.ext.intersphinx",
    "sphinx.ext.todo",
    "sphinx.ext.coverage",
    "sphinx.ext.mathjax",
    "sphinx.ext.ifconfig",
    "sphinx.ext.viewcode",
    "sphinxcontrib.srclinks",
    "sphinx_rtd_theme",
    "m2r",
]

if os.getenv("USE_DOXYREST"):
    extensions += ["doxyrest", "cpplexer"]
else:
    extensions += ["breathe", "exhale"]
    breathe_projects = {
        "cmsgemos": "../exhalebuild/xml/",
    }

    breathe_default_project = "cmsgemos"

    # Setup the exhale extension
    exhale_args = {
        # These arguments are required
        "containmentFolder": "./exhale-api",
        "rootFileName": "api.rst",
        "rootFileTitle": "``cmsgemos`` API documentation",
        "doxygenStripFromPath": "..",
        # Suggested optional arguments
        "createTreeView": True,
        # TIP: if using the sphinx-bootstrap-theme, you need
        "treeViewIsBootstrap": True,
        "exhaleExecutesDoxygen": True,
        "exhaleDoxygenStdin": """
PROJECT_NAME = cmsgemos
PROJECT_NUMBER = {}
INPUT = ../../gembase/include \
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

# Tell sphinx what the primary language being documented is.
primary_domain = "cpp"

# Tell sphinx what the pygments highlight language should be.
highlight_language = "cpp"

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = ".rst"

# The master toctree document.
master_doc = "index"

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = None

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This patterns also effect to html_static_path and html_extra_path
exclude_patterns = []

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = "sphinx"

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = True

autodoc_mock_imports = ["amc13", "uhal", "reg_utils", "xhal.reg_interface_gem"]

# -- Options for HTML output ----------------------------------------------

html_context = {
    "display_gitlab": True,
    "gitlab_host": "gitlab.cern.ch",
    "gitlab_user": "cms-gem-daq-project",
    "gitlab_repo": "cmsgemos",
    "gitlab_version": "release/legacy-1.0",  ## grab this dynamically
    "conf_py_path": "/doc/source/",
}

html_show_sourcelink = True

# The name for this set of Sphinx documents.  If None, it defaults to
# "<project> v<release> documentation".
html_title = "cmsgemos: Online Software for the CMS GEM project"

# A shorter title for the navigation bar.  Default is the same as html_title.
html_short_title = "cmsgemos"

## Set an image file to be used as a logo at the top of the sidebar
# html_logo

## Show only the logo at the top of the sidebar
# logo_only

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "sphinx_rtd_theme"

html_theme_options = {
    "navigation_depth": 50,
}

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
# html_theme_options = {}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]
# html_extra_path = ["../build/html"]

# Custom CSS
# These paths are either relative to html_static_path
# or fully qualified paths (eg. https://...)
html_css_files = [
    # "{}/css/rtd-custom.css".format(os.getenv("GEM_DOCS_URL")),
    # "css/custom.css",
]

# Custom JavaScript
# These paths are either relative to html_static_path
# or fully qualified paths (eg. https://...)
html_js_files = [
    # 'js/custom.js',
]

# # Custom sidebar templates, must be a dictionary that maps document names
# # to template names.
# #
# # This is required for the alabaster theme
# # refs: http://alabaster.readthedocs.io/en/latest/installation.html#sidebars
# html_sidebars = {
#     "**": [
#         "about.html",
#         "navigation.html",
#         "relations.html",  # needs 'show_related': True theme option to display
#         "searchbox.html",
#         "donate.html",
#     ]
# }


# -- Options for HTMLHelp output ------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = "cmsgemosdoc"


# -- Options for LaTeX output ---------------------------------------------

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    #
    # 'papersize': 'letterpaper',
    # The font size ('10pt', '11pt' or '12pt').
    #
    # 'pointsize': '10pt',
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


# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [(master_doc, "cmsgemos", u"cmsgemos Documentation", [author], 1)]


# -- Options for Texinfo output -------------------------------------------

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


# -- Options for Epub output ----------------------------------------------

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


# Example configuration for intersphinx: refer to the Python standard library.
intersphinx_mapping = {
    "python": ("https://docs.python.org/", None),
    "gemplotting": (os.getenv("EOS_SITE_URL") + "/docs/api/gemplotting/latest", None,),
    "vfatqc": (os.getenv("EOS_SITE_URL") + "/docs/api/vfatqc/latest", None,),
    "ctp7_modules": (
        os.getenv("EOS_SITE_URL") + "/docs/api/ctp7_modules/latest",
        None,
    ),
    "reg_utils": (os.getenv("EOS_SITE_URL") + "/docs/api/reg_utils/latest", None,),
    "xhal": (os.getenv("EOS_SITE_URL") + "/docs/api/xhal/latest", None,),
    "reg_interface_gem": (
        os.getenv("EOS_SITE_URL") + "/docs/api/reg_interface_gem/latest",
        None,
    ),
    "reedmuller-c": (
        os.getenv("EOS_SITE_URL") + "/docs/api/reedmuller-c/latest",
        None,
    ),
}
