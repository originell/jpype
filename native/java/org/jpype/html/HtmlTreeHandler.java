package org.jpype.html;

import java.util.LinkedList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

/**
 * HTML document handler which creates an HTML tree.
 */
public class HtmlTreeHandler implements HtmlHandler
{

  final Document root;
  LinkedList<Element> elementStack = new LinkedList<>();
  Node current;
  Pattern attribPattern1 = Pattern.compile("^\\s*([A-z:_][A-z0-9:_.-]*)\\s*=\\s*\"([^<\"]*)\"\\s*");
  Pattern attribPattern2 = Pattern.compile("^\\s*([A-z:_][A-z0-9:_.-]*)\\s*");

  public HtmlTreeHandler()
  {
    try
    {
      DocumentBuilder db = DocumentBuilderFactory.newInstance().newDocumentBuilder();
      root = db.newDocument();
      current = root;
    } catch (ParserConfigurationException ex)
    {
      throw new RuntimeException(ex);
    }
  }

  @Override
  public void startElement(String name, String attr)
  {
    name = name.toLowerCase();
    Element elem = root.createElement(name);
    if (attr != null)
    {
      while (!attr.isEmpty())
      {
        Matcher m = attribPattern1.matcher(attr);
        if (!m.find())
        {
          break;
        }
        attr = m.replaceFirst("");
        elem.setAttribute(m.group(1), m.group(2));
      }
      // Handle boolean attributes
      while (!attr.isEmpty())
      {
        Matcher m = attribPattern2.matcher(attr);
        if (!m.find())
        {
          break;
        }
        attr = m.replaceFirst("");
        elem.setAttribute(m.group(1), m.group(1));
      }

      if (!attr.isEmpty())
      {
        throw new RuntimeException("Bad attr " + attr);
      }
    }
    current.appendChild(elem);
    if (Html.VOID_ELEMENTS.contains(name))
      return;
    current = elem;
    elementStack.add(elem);
  }

  @Override
  public void endElement(String name)
  {
    name = name.toLowerCase();
    if (elementStack.isEmpty())
      throw new RuntimeException("Empty stack");
    Element last = elementStack.getLast();
    // Handle auto class tags
    while (!last.getNodeName().equals(name) && Html.OPTIONAL_ELEMENTS.contains(last.getNodeName()))
    {
      endElement(last.getNodeName());
      last = elementStack.getLast();
    }
    if (!last.getNodeName().equals(name))
    {
      throw new RuntimeException("mismatch element " + name + " " + last.getNodeName());
    }
    elementStack.removeLast();
    if (elementStack.isEmpty())
      current = root;
    else
      current = elementStack.getLast();
  }

  @Override
  public void comment(String contents)
  {
    if (contents.equals(">"))
      throw new RuntimeException();
    current.appendChild(root.createComment(contents));
  }

  @Override
  public void text(String text)
  {
    if (current == root)
      return;
    current.appendChild(root.createTextNode(text));
  }

  @Override
  public void cdata(String text)
  {
    current.appendChild(root.createCDATASection(text));
  }

  @Override
  public void startDocument()
  {
  }

  @Override
  public void endDocument()
  {
  }

  @Override
  public Object getResult()
  {
    return root;
  }

  @Override
  public void directive(String content)
  {
    int i = content.indexOf(" ");
    current.appendChild(root.createProcessingInstruction(content.substring(0, i),
            content.substring(i).trim()));
  }

}