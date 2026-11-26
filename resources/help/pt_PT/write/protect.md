# Proteger um documento e bloquear o controlo de alterações
<!-- keywords: proteger, proteção, restringir edição, só de leitura, palavra-passe, bloquear controlo de alterações, opções de controlo de alterações, nome de utilizador, iniciais, apenas alterações controladas, apenas comentários, parar proteção, desbloquear, reter, rubrica -->

A proteção limita o que as pessoas podem fazer num documento. Estas ferramentas encontram-se no separador **Rever**.

## Restringir edição

1. Clique em **Restringir Edição** no grupo **Proteger**.
2. Escolha o que as pessoas podem fazer:
   - **Sem alterações (só de leitura)**: o texto não pode ser alterado.
   - **Alterações controladas**: as pessoas podem editar, mas cada edição fica registada, e o Controlo de Alterações não pode ser desativado.
   - **Comentários**: as pessoas podem adicionar comentários, mas não podem alterar o texto.
   - **Preenchimento de formulários**: o texto é só de leitura, mas as pessoas podem preencher os campos de formulário que contém.
3. Escreva uma palavra-passe, duas vezes. Pode deixar a palavra-passe vazia. Nesse caso, qualquer pessoa pode parar a proteção.
4. Clique em **Iniciar Imposição**.

A barra de título passa então a indicar "Protegido". Para parar, clique novamente em **Restringir Edição** e, depois, em **Parar Proteção**. Escreva a palavra-passe, se existir.

Uma palavra-passe evita que pessoas de boa-fé alterem o documento por engano. Não é uma segurança forte. Não use a proteção para ocultar informações privadas.

A palavra-passe fica guardada no ficheiro como um número codificado, designado por hash. A própria palavra-passe nunca é guardada. Se a esquecer, o JeffWrite não a consegue recuperar.

## Preencher um formulário

Um documento pode ter campos de formulário: caixas para escrever, caixas de verificação e listas para escolher. O JeffWrite preenche-os, mas não adiciona novos.

- **Escrever num campo.** Clique nele e escreva. Um campo pode aceitar apenas um número limitado de letras, converter o que escreve em maiúsculas, ou aceitar apenas um número ou uma data. O JeffWrite cumpre essas regras ao escrever, colar ou introduzir texto com um método de entrada. O texto colado que seja cortado para caber mantém as primeiras letras. Uma quebra de linha no texto colado torna-se um espaço. Uma quebra de linha que já esteja no campo mantém-se, a não ser que a selecione e a substitua por escrita; a escrita nunca acrescenta uma. Um número ou uma data são apresentados no formato definido quando sai do campo: prima Tab, prima Enter, clique noutro sítio, saia com as teclas de seta, guarde ou imprima.
- **Marcar uma caixa de verificação.** Clique nela ou desloque-se até ela e prima Espaço.
- **Escolher numa lista.** Clique nela ou desloque-se até ela e prima Enter. Depois, escolha uma entrada. Ao premir uma letra, é selecionada a entrada seguinte que começa por essa letra.
- **Deslocar-se entre campos.** Quando o documento está protegido para preenchimento de formulários, prima Tab ou Enter para ir ao campo seguinte, e Shift+Tab para voltar atrás. Em qualquer documento, Enter dentro de um campo não inicia um novo parágrafo. Apenas os campos podem ser alterados. Uma parte do documento que quem criou o formulário deixou fora da proteção também pode ser alterada; nessa parte, Tab é a tecla normal, e Substituir Tudo altera apenas as correspondências dessa parte, indica quantas deixou por alterar e informa que o texto de cabeçalhos, rodapés, notas e caixas de texto ficou como está. O que escreve, cola, arrasta ou corrige é verificado pelo sítio onde iria ficar, e não pelo sítio onde está o cursor. Imagens, formas e tabelas podem ser adicionadas e alteradas numa parte deixada fora da proteção, como em qualquer documento, e não são alteradas em mais nenhum lado.
- **Anular.** Prima Anular. As letras que escreve num campo contam como um passo. Anular desfaz os passos dados desde o início da proteção, ou seja, as suas entradas e as suas alterações numa parte deixada fora da proteção, e nada de antes.

Um campo que esteja desativado no formulário, ou que o formulário calcule sozinho, não pode ser preenchido. Em qualquer documento, as marcas de início, de fim e outras marcas de um campo nunca são eliminadas uma a uma: selecione o campo inteiro para o eliminar. Um campo num documento só de leitura, ou aberto apenas para comentários, também não pode ser preenchido. Com o Controlo de Alterações ativado num documento que não está protegido, o que escreve num campo de texto fica registado como alteração, e o limite de caracteres e as maiúsculas do campo não são aplicados.

## Bloquear o controlo de alterações

Clique em **Opções de Controlo de Alterações** e, depois, em **Bloquear Controlo de Alterações**. Todas as alterações feitas a partir desse momento ficam registadas. Ninguém pode desativar o Controlo de Alterações, nem aceitar ou rejeitar uma alteração, sem a palavra-passe. É o mesmo que proteger o documento para alterações controladas.

## Alterar o seu nome nas alterações

Clique em **Opções de Controlo de Alterações** e, depois, em **Opções de Controlo de Alterações**. Escreva o seu nome e as suas iniciais. Estes identificam as alterações e os comentários que fizer a partir desse momento.
