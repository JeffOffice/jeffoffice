# Trabalhar com arquivos .pub

<!-- keywords: pub, .pub, arquivo pub, abrir pub, salvar pub, converter, compatibilidade, importar, publicações antigas, outros programas, jpub, formato, fontes ausentes, fazer backup do original, formato de arquivo -->

Um arquivo .pub é uma publicação criada por outros programas de editoração eletrônica. O JeffPub abre arquivos .pub criados desde 1998. Ele também pode salvar o seu trabalho como arquivo .pub. O formato próprio do JeffPub é o .jpub.

## Abrir um arquivo .pub

1. Pressione Ctrl+O ou clique em **Arquivo > Abrir**.
2. Clique em **Procurar**, escolha o arquivo .pub e clique em **Abrir**.

Você também pode arrastar o arquivo para a página. Textos, imagens, formas e tabelas são trazidos junto com as caixas de texto vinculadas. A maioria das páginas fica parecida com o original, mas nem todos os detalhes correspondem ainda. Algumas caixas de texto podem terminar uma ou duas palavras antes ou depois do que terminavam antes.

Se uma fonte do arquivo não estiver no seu computador, o JeffPub usa uma fonte aberta parecida. As fontes armazenadas dentro do arquivo .pub não são importadas. Se o JeffPub tiver algo a informar depois de abrir o arquivo, uma nota aparece na barra de status, na parte inferior da janela.

Se o arquivo não abrir, o JeffPub informa que ele pode estar danificado ou usar recursos que ele ainda não consegue ler. O mesmo vale para um arquivo que contém textos ou formas, mas nenhum deles pode ser colocado em uma página: ele não é aberto como páginas em branco, para que você não o sobrescreva por engano, e o arquivo permanece como estava.

## Verificar o resultado

Abrir o arquivo é uma conversão, então confira todas as páginas. Verifique quebras de linha, fontes e imagens. Use o [Verificador de Design](design-checker) para encontrar textos que não cabem.

## Salvar como arquivo .pub

- Clique em **Arquivo > Exportar** e depois em **Salvar como Arquivo .pub**.
- Ou clique em **Arquivo > Salvar Como** e escolha **Arquivos de Publicação .pub (*.pub)** em **Salvar como tipo**.

Ao salvar um arquivo .pub, o JeffPub pode gravar páginas, caixas de texto e caixas de texto vinculadas, formatação de texto, formas, linhas, imagens, tabelas, preenchimentos, sombras, Texto Artístico, estilos, livretos, cartões dobrados, envelopes, números de página e links. Se algo não tiver forma em .pub, o JeffPub informa quantos objetos deixou de fora. Esses objetos continuam na sua publicação e são mantidos quando você salva como arquivo .jpub. Os comentários também não têm forma em .pub: o JeffPub informa que os deixou de fora, e eles são mantidos apenas em arquivos .jpub. O tópico Comentários desta Ajuda traz mais informações.

## Proteger o original

Ao abrir um arquivo .pub e pressionar Ctrl+S, o JeffPub grava as alterações nesse mesmo arquivo .pub. Para manter o original seguro:

- Ative **Sempre criar cópia de backup** em **Arquivo > Opções**, na guia **Salvar**. O JeffPub então mantém o arquivo anterior com "Backup de" seguido do nome original.
- Ou comece com **Arquivo > Salvar Como** e salve um arquivo .jpub. Um arquivo .jpub mantém tudo.

Guarde os arquivos originais até ter certeza de que os novos estão corretos.

## Relacionados

- [Abrir e salvar publicações](open-save)
- [Salvar como imagem, página da Web ou e-book](export)
- [Alterar as configurações do JeffPub](options)
