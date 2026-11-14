# Separador Rever
<!-- keywords: separador rever, friso, verificação, ortografia, dicionário de sinónimos, contagem de palavras, comentários, novo comentário, eliminar comentário, anterior, seguinte, mostrar comentários, idioma, idioma de verificação, definir idioma, não verificar ortografia, traduzir, tradução, libretranslate, gramática, verificação gramatical, estilo, linha azul, linha pontilhada, verificar gramática ao escrever -->

O separador **Rever** verifica as suas palavras, traduz o texto e gere os comentários.

- **Verificação** tem **Ortografia** (F7), **Dicionário de Sinónimos** (Shift+F7), **Contagem de Palavras** e **Verificar Acessibilidade**. Consulte [Escrever e formatar texto](text).
- **Idioma** tem **Traduzir**, **Definir Idioma de Verificação** e **Não Verificar Ortografia**. Consulte mais abaixo.
- **Comentários** tem **Novo Comentário** (Ctrl+Alt+M), **Eliminar** com o respetivo menu, **Anterior**, **Seguinte** e **Mostrar Comentários**. Consulte [Comentários](comments).

## Verificar a gramática à medida que escreve

O JeffDeck também procura erros gramaticais e de estilo evidentes no texto, nos diapositivos e nas notas do orador. Um problema fica com uma linha azul pontilhada por baixo, e uma palavra com erro ortográfico mantém a linha vermelha ondulada. Clique com o botão direito no problema para ver o que está errado e escolher uma sugestão, ou escolha **Ignorar Este Problema** ou **Ignorar Tudo**. A **Ortografia** (F7) passa aos problemas de gramática dos diapositivos depois da ortografia. O texto num idioma que não é verificado, e o texto marcado como "não verificar", é ignorado. Para desativar esta opção, clique em **Ficheiro > Opções > Verificação** e desmarque **Verificar a gramática à medida que escreve**. É uma opção única para todas as aplicações JeffOffice.

**Idiomas verificados.** Inglês, francês, alemão e espanhol, cada um com regras próprias. O francês verifica o espaço sem quebra antes de ; : ! ? e dentro de « », o apóstrofo em palavras como "l'ami" e "qu'il", e confusões como "a" e "à". O alemão verifica "das" e "dass", "seid" e "seit", "wider" e "wieder", a vírgula antes de "dass" e "weil", e as maiúsculas em substantivos comuns. O espanhol procura o ¿ ou o ¡ em falta, os acentos em palavras interrogativas, "a ver" e "haber", e "del" e "al". Um parágrafo é verificado no idioma em que está marcado, e o texto sem idioma é verificado como inglês. Num parágrafo com mais do que um idioma, é usado o primeiro idioma verificado e o restante é ignorado.

**Escolher o que é verificado.** Clique em **Ficheiro > Opções > Verificação > Definições de gramática** para escolher os tipos de problemas: **Gramática** (verbos que não concordam, forma errada de um verbo, "a" e "an"), **Pontuação** (espaços, vírgulas e hífenes a mais ou em falta), **Maiúsculas**, **Palavras confundidas** (palavras que soam parecido), **Repetição** e **Estilo**. O **Estilo** está desativado até o ativar. Não indica erros, mas sim sugestões: frases com mais de 40 palavras, a voz passiva, três frases seguidas que começam pela mesma palavra e expressões com muitas palavras ("in order to", "due to the fact that"). A escolha mantém-se ao clicar em **OK** nas Opções. Aplica-se de imediato às linhas azuis e ao painel Ortografia (F7) nas janelas abertas desta aplicação, e às restantes aplicações JeffOffice na próxima vez que forem iniciadas.

## Definir o idioma de algum texto

A Ortografia verifica cada parte do texto no seu próprio idioma. Para alterar o idioma de algumas palavras, selecione-as, ou selecione uma caixa de texto ou um marcador de posição completo, e clique em **Definir Idioma de Verificação** no grupo **Idioma**. Escolha o idioma na lista e clique em **OK**. As palavras nesse idioma são verificadas com o dicionário desse idioma, e o ficheiro mantém esta escolha ao guardá-lo como apresentação ou como apresentação OpenDocument. O texto que não tiver marcado é verificado em inglês americano.

## Não verificar algum texto

Há texto que não deve ser verificado: um código, um nome ou uma linha numa língua inventada. Selecione-o e clique em **Não Verificar Ortografia** no grupo **Idioma**. As linhas vermelhas onduladas desaparecem. Clique novamente para verificar o texto outra vez. A caixa de notas contém apenas texto simples, por isso não tem idioma próprio.

## Traduzir texto

1. Selecione as palavras numa caixa de texto ou nas notas. Para traduzir tudo numa caixa de texto, selecione a caixa.
2. Clique em **Traduzir** no grupo **Idioma**.
3. Escolha o idioma para o qual pretende traduzir.
4. Clique em **OK**.

O seu navegador abre então um serviço de tradução chamado LibreTranslate, com o seu texto. Nada é enviado até clicar em **OK**. Pode alterar o endereço do serviço na caixa, por exemplo para um que execute no seu próprio computador. São utilizados apenas endereços Web seguros (https), e endereços http para este computador. São enviadas, no máximo, 5000 letras.

## Verificar acessibilidade

**Verificar Acessibilidade** abre uma pequena janela que lista o que pode ser difícil de utilizar para algumas pessoas: uma imagem ou forma sem texto alternativo, um diapositivo sem título, uma tabela sem linha de cabeçalho, um vídeo sem legendas, texto difícil de ler sobre o respetivo fundo, e objetos que um leitor de ecrã lê numa ordem diferente da que a respetiva posição sugere. O texto precisa de um contraste de 4,5 para 1 com o fundo (3 para 1 para texto de 18 pontos ou mais, ou 14 pontos ou mais a negrito). Quando o texto está sobre uma imagem, uma sugestão pede-lhe que verifique o contraste a olho. Um leitor de ecrã lê os objetos de um diapositivo desde o fundo da pilha até à frente. Clique em **Ordem de Leitura** para os ver nessa ordem, e use **Ler Antes** e **Ler Depois** (ou arraste) para a alterar; cada movimento corresponde a um passo de **Anular**. Um leitor de ecrã usa a primeira linha de uma tabela para dar nome às colunas: selecione o problema da tabela e clique em **Usar a primeira linha como cabeçalho** para ativar a linha de cabeçalho (um passo de **Anular**). Um vídeo sem legendas aparece como sugestão, porque os vídeos precisam de legendas para as pessoas que não ouvem. Não há correção com um só clique: selecione o vídeo e use **Inserir Legendas** no separador **Reprodução**, e a sugestão desaparece. Clique num problema para ir ao respetivo diapositivo e selecionar o objeto. Dê à imagem um texto alternativo no painel **Formatar Imagem**, ou marque-a como decorativa, e clique em **Verificar Novamente**. Também pode iniciar a verificação a partir de **Ficheiro > Informações**.

## Relacionados

- [Comentários](comments)
- [Escrever e formatar texto](text)
